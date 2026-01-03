#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;

#include "include/ssbo_entities.glsl"
#include "include/ssbo_instance_indeces.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_clip_planes.glsl"

out VS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

#include "include/fn_calculate_clipping.glsl"

void main() {
  instance = u_instances[gl_BaseInstance + gl_InstanceID];
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include "include/var_entity_model_matrix.glsl"
  #include "include/var_entity_normal_matrix.glsl"

  const uint umaterialIndex = instance.u_materialIndex;
  const vec4 worldPos = modelMatrix * vec4(a_pos, 1.0);

  gl_Position = u_projectedMatrix * worldPos;

  calculateClipping(worldPos);

  vs_out.entityIndex = gl_BaseInstance + gl_InstanceID;
  vs_out.worldPos = worldPos.xyz;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * DECODE_A_NORMAL(a_normal);
}
