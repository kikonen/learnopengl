#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_entity.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl
#include ssbo_materials.glsl
#include ssbo_material_indeces.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl

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
  const uint entityIndex = u_instances[gl_BaseInstance + gl_InstanceID];
  entity = u_entities[entityIndex];

  #include var_entity_model_matrix.glsl

  int materialIndex = entity.u_materialIndex;

  vs_out.materialIndex = materialIndex;

  vs_out.texCoord.x = a_texCoord.x;
  vs_out.texCoord.y = 1.0 - a_texCoord.y;

  gl_Position = u_projectedMatrix * modelMatrix * vec4(a_pos, 1.0);
}
