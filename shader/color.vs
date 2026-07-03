#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;

#include "include/tbo_entities.glsl"
#include "include/tbo_instances.glsl"

#include "include/ssbo_instance_indeces.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

#include "include/fn_fill_instance_tbo.glsl"
#include "include/fn_fill_entity_tbo.glsl"

void main() {
  const uint instanceIndex = GET_INSTANCE_INDEX;
  fillInstanceTBO(instanceIndex);

  const uint entityIndex = instance.u_entityIndex;
  fillEntityTBO(entityIndex);

  #include "include/var_entity_model_matrix.glsl"

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos = modelMatrix * pos;

  gl_Position = u_projectedMatrix * worldPos;
}
