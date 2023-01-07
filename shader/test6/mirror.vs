#version 460 core

#include constants.glsl

layout (location = ATTR_POS) in vec4 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;
//layout (location = ATTR_MATERIAL_INDEX) in float a_materialIndex;
//layout (location = ATTR_INSTANCE_MODEL_MATRIX_1) in mat4 a_modelMatrix;
//layout (location = ATTR_INSTANCE_NORMAL_MATRIX_1) in mat3 a_normalMatrix;
layout (location = ATTR_INSTANCE_ENTITY_INDEX) in float a_entityIndex;

#include struct_material.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_materials.glsl
#include uniform_data.glsl


out VS_OUT {
  vec4 glp;

  vec3 fragPos;
  vec3 normal;
  vec2 texCoord;
  vec4 vertexPos;
  vec3 viewVertexPos;

  flat uint materialIndex;

  vec4 fragPosLightSpace;

  mat3 TBN;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  Entity entity = u_entities[int(a_entityIndex)];
  mat3 normalMatrix = mat3(entity.normalMatrix);
  int materialIndex = int(entity.materialIndex);
  vec4 worldPos = entity.modelMatrix * a_pos;

  vs_out.glp = u_projectedMatrix * worldPos;
  gl_Position = vs_out.glp;

  vs_out.materialIndex = materialIndex;
  vs_out.texCoord = a_texCoord;

  vs_out.fragPos = worldPos.xyz;
  vs_out.vertexPos = a_pos;
  vs_out.viewVertexPos = (u_viewMatrix * worldPos).xyz;

  vs_out.normal = normalize(normalMatrix * a_normal);

  vs_out.fragPosLightSpace = u_shadowMatrix * worldPos;
}
