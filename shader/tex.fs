#version 460 core

#include struct_lights.glsl
#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl
#include uniform_textures.glsl

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
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  flat uint shadowIndex;
  vec4 shadowPos;
#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;
layout(binding = UNIT_SHADOW_MAP_FIRST) uniform sampler2DShadow u_shadowMap[MAX_SHADOW_MAP_COUNT];

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION

#include fn_calculate_dir_light.glsl
#include fn_calculate_point_light.glsl
#include fn_calculate_spot_light.glsl
#include fn_calculate_light.glsl
#include fn_calculate_normal_pattern.glsl
#include fn_calculate_fog.glsl

void main() {
  const vec3 toView = normalize(u_viewWorldPos - fs_in.worldPos);
  #include var_tex_material.glsl

#ifdef USE_ALPHA
#ifdef USE_BLEND_OIT
  if (material.diffuse.a < 0.95)
    discard;
#else
  if (material.diffuse.a < 0.01)
    discard;
#endif
#endif

  #include var_tex_material_normal.glsl

  if (material.pattern == 1) {
    normal = calculateNormalPattern(fs_in.vertexPos, normal);
  }

  // if (!gl_FrontFacing) {
  //   normal = -normal;
  // }

  #include var_calculate_cubemap_diffuse.glsl

  vec4 texColor = calculateLight(normal, toView, fs_in.worldPos, fs_in.shadowIndex, fs_in.shadowPos, material);

#ifdef USE_ALPHA
#ifdef USE_BLEND_OIT
  if (material.diffuse.a < 0.95)
    discard;
#else
  if (material.diffuse.a < 0.01)
    discard;
#endif
#endif

#ifndef USE_BLEND
  texColor = vec4(texColor.rgb, 1.0);
#endif

  // if (!gl_FrontFacing) {
  //   float alpha = texColor.a;
  //   texColor = mix(texColor, vec4(0.1, 0.1, 0.9, 1.0), 0.15);
  //   texColor.a = alpha;
  // }

  texColor = calculateFog(fs_in.viewPos, texColor);

  o_fragColor = texColor;
}
