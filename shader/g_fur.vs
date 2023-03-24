#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_NORMAL_TEX
layout (location = ATTR_TANGENT) in vec3 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_material_indeces.glsl

out VS_OUT {
  mat4 modelMatrix;
  mat3 normalMatrix;

  vec3 normal;
#ifdef USE_NORMAL_TEX
  vec3 tangent;
#endif
  vec2 texCoord;

  flat uint materialIndex;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  vs_out.modelMatrix = modelMatrix;
  vs_out.normalMatrix = normalMatrix;

  int materialIndex = entity.materialIndex;

  if (materialIndex < 0) {
    materialIndex = u_materialIndeces[-materialIndex + gl_VertexID - gl_BaseVertex];
  }

  gl_Position = vec4(a_pos, 1.0);

  vs_out.materialIndex = materialIndex;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = a_normal;

#ifdef USE_NORMAL_TEX
  if (u_materials[materialIndex].normalMapTex >= 0)
  {
    vs_out.tangent = a_tangent;
  }
#endif
}
