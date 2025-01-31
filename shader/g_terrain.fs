#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_debug.glsl
#include uniform_data.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in TES_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
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

  float height;
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include fn_gbuffer_encode.glsl
#ifdef USE_PARALLAX
#include fn_calculate_parallax_mapping.glsl
#endif

void main() {
  const uint materialIndex = fs_in.materialIndex;

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

  #include var_tex_material_normal.glsl

  // if (!gl_FrontFacing) {
  //   normal = -normal;
  // }

#ifdef USE_CUBE_MAP
  const vec3 viewDir = normalize(u_cameraPos - fs_in.worldPos);

  #include var_calculate_cube_map_diffuse.glsl
#endif

  vec4 texColor = material.diffuse;

  o_fragColor = texColor.rgb;
  o_fragMRA = material.mra;
  o_fragEmission = material.emission;

  //o_fragPosition = fs_in.worldPos;
  o_fragNormal = encodeGNormal(normal, fs_in.viewPos);
}
