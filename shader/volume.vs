#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;

#include "include/tbo_entities.glsl"
#include "include/tbo_instances.glsl"
#include "include/tbo_materials.glsl"

#include "include/ssbo_instance_indeces.glsl"
#include "include/ssbo_socket_transforms.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"

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

#include "include/fn_fill_instance_tbo.glsl"
#include "include/fn_fill_entity_tbo.glsl"

void main() {
  const uint instanceIndex = GET_INSTANCE_INDEX;
  fillInstanceTBO(instanceIndex);

  const uint entityIndex = instance.u_entityIndex;
  fillEntityTBO(entityIndex);

  #include "include/var_entity_model_matrix.glsl"
  #include "include/var_entity_normal_matrix.glsl"

  uint materialIndex = instance.u_materialIndex;
  const vec4 worldPos = modelMatrix * vec4(a_pos, 1.0);

  gl_Position = u_projectedMatrix * worldPos;
  vs_out.worldPos = worldPos.xyz;

  vs_out.materialIndex = materialIndex;
}
