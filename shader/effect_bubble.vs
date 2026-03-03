#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;

#include "include/ssbo_entities.glsl"
#include "include/ssbo_instances.glsl"
#include "include/ssbo_instance_indeces.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_clip_planes.glsl"

out VS_OUT {
  flat uint materialIndex;
  vec3 viewPos;
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
  instance = GET_INSTANCE;
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include "include/var_entity_model_matrix.glsl"
  #include "include/var_entity_normal_matrix.glsl"

  const uint materialIndex = instance.u_materialIndex;
  const vec4 pos = vec4(a_pos, 1.0);
  const vec4 viewPos = u_viewMatrix * modelMatrix * pos;

  gl_Position = u_projectionMatrix * viewPos;

  // calculateClipping(worldPos);

  vs_out.materialIndex = materialIndex;

  vs_out.viewPos = vec3(viewPos);

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * DECODE_A_NORMAL(a_normal);

  // NOTE KI normal in viewSpace
  // 1) this is same as 2
  // mat3 normalMat = mat3(transpose(inverse(u_viewMatrix * modelMatrix)));
  // vs_out.normal = normalize(normalMat * DECODE_A_NORMAL(a_normal));
  // 2)
  vs_out.normal = normalize(mat3(u_viewMatrix) * normalMatrix * DECODE_A_NORMAL(a_normal));
}
