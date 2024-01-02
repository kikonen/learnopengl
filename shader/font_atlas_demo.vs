#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_material_indeces.glsl


out VS_OUT {
  vec2 texCoord;

  flat uint materialIndex;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Entity entity;

void main() {
  entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl

  int materialIndex = entity.u_materialIndex;

  vs_out.materialIndex = materialIndex;

  vs_out.texCoord.x = a_texCoord.x;
  vs_out.texCoord.y = a_texCoord.y;

  gl_Position = u_projectedMatrix * modelMatrix * vec4(a_pos, 1.0);
}
