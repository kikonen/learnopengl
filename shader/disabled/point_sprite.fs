#version 460 core

#include include/struct_lights.glsl
#include include/struct_material.glsl

#include include/uniform_matrices.glsl
#include include/uniform_data.glsl
#include include/uniform_lights.glsl
#include include/ssbo_materials.glsl
#include include/uniform_textures.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in GS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  flat uint shadowIndex;
  vec4 shadowPos;

#ifdef USE_TBN
  vec3 tangent;
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;
layout(binding = UNIT_SHADOW_MAP_FIRST) uniform sampler2DShadow u_shadowMap[MAX_SHADOW_MAP_COUNT];

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Material material;

#include include/fn_calculate_dir_light.glsl
#include include/fn_calculate_point_light.glsl
#include include/fn_calculate_spot_light.glsl
#include include/fn_calculate_light.glsl
#include include/fn_calculate_normal_pattern.glsl
#include include/fn_calculate_fog.glsl

void main() {
  material = u_materials[fs_in.materialIndex];

  vec2 texCoord = fs_in.texCoord;
  #include include/apply_parallax.glsl

  #include include/var_tex_material.glsl

  const vec3 viewDir = normalize(u_cameraPos.xyz - fs_in.worldPos);

#ifdef USE_ALPHA
  if (material.diffuse.a < 0.01)
    discard;
#endif

  #include include/var_tex_material_normal.glsl

#ifdef USE_CUBE_MAP
  #include include/var_calculate_cube_map_diffuse.glsl
#endif

  vec4 texColor = calculateLight(
    normal, viewDir, fs_in.worldPos,
    fs_in.shadowIndex,
    material);

#ifdef USE_ALPHA
  if (texColor.a < 0.1)
    discard;
#endif

#ifndef USE_BLEND
  texColor = vec4(texColor.rgb, 1.0);
#endif

  texColor = calculateFog(fs_in.viewPos, texColor);
  o_fragColor = texColor;
}
