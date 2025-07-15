#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_resolved_material.glsl

#include struct_entity.glsl
#include struct_instance.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl
#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl

out VS_OUT {
  flat mat4 modelMatrix;

  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  flat int layers;
  flat float layersDepth;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

void main() {
  instance = u_instances[gl_BaseInstance + gl_InstanceID];
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include var_entity_model_matrix.glsl
  #include var_entity_normal_matrix.glsl

  vs_out.modelMatrix = modelMatrix;

  const uint materialIndex = instance.u_materialIndex;

  vs_out.materialIndex = materialIndex;

  vs_out.layers = u_materials[materialIndex].layers;
  vs_out.layersDepth = u_materials[materialIndex].layersDepth;

  vs_out.texCoord.x = a_texCoord.x * u_materials[materialIndex].tilingX * entity.tilingX;
  vs_out.texCoord.y = a_texCoord.y * u_materials[materialIndex].tilingY * entity.tilingY;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = DECODE_A_NORMAL(a_normal);//normalMatrix * DECODE_A_NORMAL(a_normal);

  gl_Position = vec4(a_pos, 1.0);
}
