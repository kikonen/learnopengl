#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_materials.glsl
#include uniform_data.glsl


out VS_OUT {
  vec4 glp;

  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  vec4 shadowPos;

  mat3 TBN;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  // TODO KI select shadow map index
  const uint shadowIndex = 0;

  const int materialIndex = entity.materialIndex;
  const vec4 pos = vec4(a_pos, 1.0);
  const vec4 worldPos = modelMatrix * pos;

  vs_out.glp = u_projectedMatrix * worldPos;
  gl_Position = vs_out.glp;

  vs_out.entityIndex = gl_BaseInstance + gl_InstanceID;
  vs_out.materialIndex = materialIndex;
  vs_out.texCoord = a_texCoord;

  vs_out.worldPos = worldPos.xyz;
  vs_out.vertexPos = a_pos;
  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * a_normal;

  vs_out.shadowPos = u_shadowMatrix[shadowIndex] * worldPos;
}
