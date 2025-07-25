#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl
#include ssbo_socket_transforms.glsl
#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  vec4 shadowPos;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

#include fn_calculate_clipping.glsl
#include fn_calculate_shadow_index.glsl

void main() {
  instance = u_instances[gl_BaseInstance + gl_InstanceID];
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const uint materialIndex = instance.u_materialIndex;
  const vec4 pos = vec4(a_pos, 1.0);
  const vec4 worldPos = modelMatrix * pos;

  const vec3 viewPos = (u_viewMatrix * worldPos).xyz;
  const uint shadowIndex = calculateShadowIndex(viewPos);

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.entityIndex = gl_BaseInstance + gl_InstanceID;
  vs_out.materialIndex = materialIndex;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX * entity.tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY * entity.tilingY;

  vs_out.worldPos = worldPos.xyz;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * DECODE_A_NORMAL(a_normal);

  calculateClipping(worldPos);

  vs_out.shadowPos = u_shadowMatrix[shadowIndex] * worldPos;
}
