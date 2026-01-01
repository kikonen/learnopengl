#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;

#include include/ssbo_entities.glsl
#include include/ssbo_instance_indeces.glsl

#include include/uniform_matrices.glsl
#include include/uniform_camera.glsl
#include include/uniform_data.glsl

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

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos = modelMatrix * pos;

  gl_Position = u_projectedMatrix * worldPos;
}
