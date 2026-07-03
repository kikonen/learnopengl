#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include "include/tbo_entities.glsl"
#include "include/tbo_instances.glsl"
#include "include/tbo_materials.glsl"

#include "include/ssbo_instance_indeces.glsl"
#include "include/ssbo_socket_transforms.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"


out VS_OUT {
  vec4 glp;

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

  const uint materialIndex = instance.u_materialIndex;
  const vec4 pos = vec4(a_pos, 1.0);
  const vec4 worldPos = modelMatrix * pos;
  const vec3 normal = normalize(viewNormalMatrix * DECODE_A_NORMAL(a_normal));

  vs_out.glp = u_projectedMatrix * worldPos;
  gl_Position = vs_out.glp;

  vs_out.materialIndex = materialIndex;

  fillMaterialTilingTBO(materialIndex);

  vs_out.texCoord.x = a_texCoord.x * material.tilingX * entity.tilingX;
  vs_out.texCoord.y = a_texCoord.y * material.tilingY * entity.tilingY;

  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  vs_out.normal = normal;
}
