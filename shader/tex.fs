#version 460 core

#define PASS_FORWARD

#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl
#include uniform_shadow.glsl
#include uniform_debug.glsl
#include uniform_lights.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 viewPos;

  flat uint materialIndex;
  flat uint flags;

  flat uint shadowIndex;
  vec4 shadowPos;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif
} fs_in;

layout(binding = UNIT_IRRADIANCE_MAP) uniform samplerCube u_irradianceMap;
layout(binding = UNIT_PREFILTER_MAP) uniform samplerCube u_prefilterMap;
layout(binding = UNIT_BRDF_LUT) uniform sampler2D u_brdfLut;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;
layout(binding = UNIT_SHADOW_MAP_FIRST) uniform sampler2DShadow u_shadowMap[MAX_SHADOW_MAP_COUNT];

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include pbr.glsl
#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_fog.glsl

#ifdef USE_PARALLAX
#include fn_calculate_parallax_mapping.glsl
#endif

void main() {
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;
  #include apply_parallax.glsl

  #include var_tex_material.glsl

  const vec3 viewDir = normalize(u_cameraPos.xyz - fs_in.worldPos);

#ifdef USE_ALPHA
  if (material.diffuse.a < ALPHA_THRESHOLD)
    discard;
#endif

  // NOTE KI interpolation from vs to fs denormalizes normal
  vec3 normal = normalize(fs_in.normal);
  #include apply_normal_map.glsl

  // if (!gl_FrontFacing) {
  //   normal = -normal;
  // }

#ifdef USE_CUBE_MAP
  #include var_calculate_cube_map_diffuse.glsl
#endif

  vec4 texColor = calculateLightPbr(
    normal, viewDir, fs_in.worldPos,
    fs_in.shadowIndex);

#ifdef USE_ALPHA
  if (material.diffuse.a < ALPHA_THRESHOLD)
    discard;
#endif

#ifndef USE_BLEND
  texColor = vec4(texColor.rgb, 1.0);
#endif

  // if (!gl_FrontFacing) {
  //   float alpha = texColor.a;
  //   texColor = mix(texColor, vec4(0.1, 0.1, 0.9, 1.0), 0.15);
  //   texColor.a = alpha;
  // }

  // texColor = calculateFog(fs_in.viewPos, texColor);

  o_fragColor = texColor;
  // o_fragColor.g = 1.0;
}
