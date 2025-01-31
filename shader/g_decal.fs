#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_debug.glsl
#include ssbo_materials.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  flat vec2 spriteCoord;
  flat vec2 spriteSize;

  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#ifdef USE_PARALLAX
#include fn_calculate_parallax_mapping.glsl
#endif
#include fn_gbuffer_encode.glsl

void main() {
  const uint materialIndex = fs_in.materialIndex;

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

#ifdef USE_ALPHA
#ifdef USE_BLEND
  if (material.diffuse.a < OIT_MAX_BLEND_THRESHOLD)
    discard;
#else
  if (material.diffuse.a < GBUFFER_ALPHA_THRESHOLD)
    discard;
#endif
#endif

  #include var_tex_material_normal.glsl

  if (!gl_FrontFacing) {
    normal = -normal;
  }

#ifdef USE_CUBE_MAP
  const vec3 viewDir = normalize(u_cameraPos - fs_in.worldPos);

  #include var_calculate_cube_map_diffuse.glsl
#endif

  vec4 color = material.diffuse;

  clamp_color(color);

  o_fragColor = color.rgb;
  o_fragMRA = material.mra;
  o_fragEmission = material.emission;

  // o_fragColor = vec3(1, 0, 0);

  //o_fragPosition = fs_in.worldPos;
  o_fragNormal = encodeGNormal(normal, fs_in.viewPos);
}
