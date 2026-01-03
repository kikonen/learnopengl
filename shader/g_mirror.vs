#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include "include/ssbo_entities.glsl"
#include "include/ssbo_instances.glsl"
#include "include/ssbo_instance_indeces.glsl"
#include "include/ssbo_socket_transforms.glsl"
#include "include/ssbo_materials.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"


out VS_OUT {
  vec4 glp;

  // vec3 worldPos;
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

void main() {
  instance = GET_INSTANCE;
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include "include/var_entity_model_matrix.glsl"
  #include "include/var_entity_normal_matrix.glsl"

  const uint materialIndex = instance.u_materialIndex;
  const vec4 pos = vec4(a_pos, 1.0);
  const vec4 worldPos = modelMatrix * pos;
  const vec3 normal = normalize(normalMatrix * DECODE_A_NORMAL(a_normal));

  vs_out.glp = u_projectedMatrix * worldPos;
  gl_Position = vs_out.glp;

  vs_out.materialIndex = materialIndex;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX * entity.tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY * entity.tilingY;

  // vs_out.worldPos = worldPos.xyz;
  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  vs_out.normal = normal;
}
