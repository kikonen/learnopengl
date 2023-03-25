#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
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

  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  flat int layers;
  flat float depth;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  const Entity entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include var_entity_model_matrix.glsl

  vs_out.modelMatrix = modelMatrix;

  int materialIndex = entity.materialIndex;
  if (materialIndex < 0) {
    materialIndex = u_materialIndeces[-materialIndex + gl_VertexID - gl_BaseVertex];
  }

  vs_out.materialIndex = materialIndex;

  vs_out.layers = u_materials[materialIndex].layers;
  vs_out.depth = u_materials[materialIndex].depth;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = a_normal;

  gl_Position = vec4(a_pos, 1.0);
}
