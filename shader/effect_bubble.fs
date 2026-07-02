#version 460 core

#define PASS_FORWARD

#include "include/tbo_materials.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_buffer_info.glsl"
#include "include/uniform_debug.glsl"

in VS_OUT {
  flat uint materialIndex;
  vec3 viewPos;
  vec3 normal;
} fs_in;

layout(binding = UNIT_G_DEPTH) uniform sampler2D g_depth;

#ifdef EFFECT_ENV_DIM
// ambient IBL, used only to dim the stylized glow by scene light level (opt-in)
layout(binding = UNIT_IRRADIANCE_MAP) uniform samplerCube u_irradianceMap;
#endif

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include "include/fn_fill_material_tbo.glsl"

#include "include/fn_util.glsl"
#include "include/fn_gbuffer_depth_decode.glsl"

void main() {
  const uint materialIndex = fs_in.materialIndex;

  fillMaterialPlainTBO(materialIndex);

  // - Rim lighting
  vec4 rim;
  {
    // NOTE KI interpolation from vs to fs denormalizes normal
    const vec3 normal = normalize(fs_in.normal);

    const vec3 viewDir = -normalize(fs_in.viewPos);

    // The more orthogonal the camera is to the fragment, the stronger the rim light.
    // abs() so that the back faces get treated the same as the front, giving a rim effect.
    // The more orthogonal, the stronger
    const float rimStrength = 1 - abs(dot(viewDir, normal));

    // higher power = sharper rim light
    const float rimFactor = pow(rimStrength, 4);
    rim = vec4(rimFactor);
  }

  vec4 intersection;
  {
    // - Create the intersection line -
    // Turn frag coord from screenspace -> NDC, which corresponds to the UV
    const float bubbleDepth = linearizeDepthFromUniform(gl_FragCoord.z);

    const vec2 pixCoord = gl_FragCoord.xy / u_bufferResolution;
    const float sceneDepth = linearizeDepthFromUniform(getDepthFromGBuffer(pixCoord));

    // linear difference in depth
    const float distance = abs(bubbleDepth - sceneDepth);

    const float threshold = 0.0003;

    // [0, threshold] -> [0, 1]
    const float normalizedDistance = clamp(distance / threshold, 0.0, 1.0);

    // white to transparent gradient
    intersection = mix(vec4(1), vec4(0), normalizedDistance);
  }

  vec4 bubbleBase = material.diffuse;

  vec4 colorOut = bubbleBase + intersection + rim;

#ifdef EFFECT_ENV_DIM
  // Opt-in (material: shared_definitions: { effect_env_dim: 1 }): scale the stylized
  // glow by ambient light level so the bubble fades in the dark instead of glowing.
  // The rim/intersection effect shape is preserved, just attenuated by the environment.
  {
    const vec3 worldNormal = normalize(mat3(u_invViewMatrix) * normalize(fs_in.normal));
    const vec3 ambient = textureLod(u_irradianceMap, worldNormal, 0).rgb;
    const float ambientLum = dot(ambient, vec3(0.2126, 0.7152, 0.0722));
    colorOut.rgb *= clamp(ambientLum, 0.0, 1.0);
  }
#endif

  o_fragColor = colorOut;
}
