#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_clip_plane.glsl
#include struct_entity.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl

#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  flat uint materialIndex;

  vec2 texCoord;
  vec3 worldPos;
  vec3 normal;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Entity entity;

#include fn_calculate_clipping.glsl

void main() {
  const uint entityIndex = u_instances[gl_BaseInstance + gl_InstanceID];
  entity = u_entities[entityIndex];

  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  const int materialIndex = entity.u_materialIndex;
  const vec4 pos = vec4(a_pos, 1.0);
  const vec4 worldPos = modelMatrix * pos;

  gl_Position = u_projectedMatrix * worldPos;

  calculateClipping(worldPos);

  vs_out.materialIndex = materialIndex;
  vs_out.texCoord = a_texCoord;

  vs_out.worldPos = worldPos.xyz;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * a_normal;
}
