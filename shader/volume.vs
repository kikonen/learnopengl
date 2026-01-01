#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;

#include include/ssbo_entities.glsl
#include include/ssbo_instance_indeces.glsl
#include include/ssbo_socket_transforms.glsl

#include include/uniform_matrices.glsl
#include include/uniform_camera.glsl

out VS_OUT {
  vec3 worldPos;
  flat uint materialIndex;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

void main() {
  instance = u_instances[gl_BaseInstance + gl_InstanceID];
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include include/var_entity_model_matrix.glsl
  #include include/var_entity_normal_matrix.glsl

  uint materialIndex = instance.u_materialIndex;
  const vec4 worldPos = modelMatrix * vec4(a_pos, 1.0);

  gl_Position = u_projectedMatrix * worldPos;
  vs_out.worldPos = worldPos.xyz;

  vs_out.materialIndex = materialIndex;
}
