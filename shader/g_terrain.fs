#version 460 core

#include "include/ssbo_materials.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_debug.glsl"

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in TES_OUT {
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
  flat uint tileIndex;
  flat uint tileX;
  flat uint tileY;

#ifdef USE_TBN
  vec4 tangent;
#endif

  float height;
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include "include/fn_gbuffer_normal_encode.glsl"
#ifdef USE_PARALLAX
#include "include/fn_calculate_parallax_mapping.glsl"
#endif

void main() {
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;

  // NOTE KI interpolation from vs to fs denormalizes normal
  vec3 normal = normalize(fs_in.normal);

  #include "include/var_calculate_tbn.glsl"
  // #include "include/apply_parallax.glsl"

  #include "include/var_tex_material.glsl"

  #include "include/apply_normal_map.glsl"

  // if (!gl_FrontFacing) {
  //   normal = -normal;
  // }

#ifdef USE_CUBE_MAP
  {
    const vec3 viewDir = -normalize(fs_in.viewPos);
#include "include/var_calculate_cube_map_diffuse.glsl"
  }
#endif

  vec4 texColor = material.diffuse;

  if ((fs_in.tileX + fs_in.tileY) % 2 == 0) {
    // texColor *= vec4(3.5, 0.7, 0.7, 1);
  }

  if (u_waterCausticEnabled) {
    vec3 worldPos = (u_invViewMatrix * vec4(fs_in.viewPos, 1)).xyz;
    if (worldPos.y < u_waterCausticWorldLevel) {
      vec2 causticTexCoord = (texCoord * 200 + vec2(sin(u_time * 0.2), cos(u_time * 0.1)) * 0.3) * 1.5;
      vec3 causticColor = texture(sampler2D(u_materials[u_waterCausticMaterialIndex].diffuseTex), causticTexCoord).rgb;

      texColor.rgb = mix(texColor.rgb, causticColor.rgb, u_waterCausticIntensity);
    }
  }

  o_fragColor = texColor.rgb;
  o_fragMRAS = material.mras;
  o_fragEmission = material.emission;

  #include "include/encode_gbuffer_normal.glsl"
  #include "include/encode_gbuffer_view_position.glsl"
  #include "include/encode_gbuffer_view_z.glsl"
}
