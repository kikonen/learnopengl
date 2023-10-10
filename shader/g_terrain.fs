#version 460 core

#include struct_material.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in TES_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

#ifdef USE_TBN
  vec3 tangent;
#endif

  float height;
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Entity entity;
Material material;

#include fn_calculate_normal_pattern.glsl
#include fn_gbuffer_encode.glsl

void main() {
  material = u_materials[fs_in.materialIndex];

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

  const vec3 viewDir = normalize(u_viewWorldPos - fs_in.worldPos);
  entity = u_entities[fs_in.entityIndex];

  #include var_tex_material_normal.glsl

#ifdef USE_NORMAL_PATTERN
  if (material.pattern == 1) {
    normal = calculateNormalPattern(fs_in.vertexPos, normal);
  }
#endif

  // if (!gl_FrontFacing) {
  //   normal = -normal;
  // }

#ifdef USE_CUBE_MAP
  #include var_calculate_cube_map_diffuse.glsl
#endif

  vec4 texColor = material.diffuse;

  o_fragColor = vec4(texColor.xyz, 1.0);
  o_fragMetal = material.metal;
  o_fragEmission = material.emission.xyz;

  //o_fragPosition = fs_in.worldPos;
  o_fragNormal = encodeGNormal(normal);
}