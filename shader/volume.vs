#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;

#include struct_entity.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl

#include uniform_matrices.glsl

out VS_OUT {
  vec3 worldPos;
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
  #include var_entity_normal_matrix.glsl

  const int materialIndex = entity.u_materialIndex;
  const vec4 worldPos = modelMatrix * vec4(a_pos, 1.0);

  gl_Position = u_projectedMatrix * worldPos;
  vs_out.worldPos = worldPos.xyz;

  vs_out.materialIndex = materialIndex;
}
