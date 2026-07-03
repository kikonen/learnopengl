#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include "include/tbo_entities.glsl"
#include "include/tbo_instances.glsl"
#include "include/tbo_materials.glsl"

#include "include/ssbo_instance_indeces.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"

out VS_OUT {
  flat mat4 modelMatrix;
  flat mat3 viewNormalMatrix;

  vec3 objectNormal;
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
ResolvedMaterial material;

#include "include/fn_fill_instance_tbo.glsl"
#include "include/fn_fill_entity_tbo.glsl"
#include "include/fn_fill_material_tbo.glsl"

void main() {
  const uint instanceIndex = GET_INSTANCE_INDEX;
  fillInstanceTBO(instanceIndex);

  const uint entityIndex = instance.u_entityIndex;
  fillEntityTBO(entityIndex);

  #include "include/var_entity_model_matrix.glsl"
  #include "include/var_entity_normal_matrix.glsl"

  vs_out.modelMatrix = modelMatrix;
  vs_out.viewNormalMatrix = viewNormalMatrix;

  const uint materialIndex = instance.u_materialIndex;
  fillMaterialDepthPeeledTBO(materialIndex);
  fillMaterialTilingTBO(materialIndex);

  vs_out.materialIndex = materialIndex;

  vs_out.layers = material.layers;
  vs_out.layersDepth = material.layersDepth;

  vs_out.texCoord.x = a_texCoord.x * material.tilingX * entity.tilingX;
  vs_out.texCoord.y = a_texCoord.y * material.tilingY * entity.tilingY;

  vs_out.objectNormal = DECODE_A_NORMAL(a_normal);

  gl_Position = vec4(a_pos, 1.0);
}
