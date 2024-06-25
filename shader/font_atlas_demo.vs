#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_entity.glsl
#include struct_instance.glsl

#include ssbo_entities.glsl
#include ssbo_instance_indeces.glsl
#include ssbo_mesh_transforms.glsl
#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl

out VS_OUT {
  vec2 texCoord;

  flat uint materialIndex;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

#include fn_decode.glsl

void main() {
  instance = u_instances[gl_BaseInstance + gl_InstanceID];
  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  #include var_entity_model_matrix.glsl

  const uint materialIndex = decodeMaterialIndex(instance.u_packedMaterial);

  vs_out.materialIndex = materialIndex;

  vs_out.texCoord.x = a_texCoord.x;
  vs_out.texCoord.y = 1.0 - a_texCoord.y;

  gl_Position = u_projectedMatrix * modelMatrix * vec4(a_pos, 1.0);
}
