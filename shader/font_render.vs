#version 460 core

#include "include/tbo_entities.glsl"
#include "include/tbo_instances.glsl"
#include "include/tbo_materials.glsl"

#include "include/ssbo_instance_indeces.glsl"
#include "include/ssbo_socket_transforms.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"

layout (location = ATTR_POS) in vec3 a_pos;
//layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;
layout (location = ATTR_FONT_ATLAS_TEX) in vec2 a_atlasCoord;

//layout(location = UNIFORM_MODEL_MATRIX) uniform mat4 u_modelMatrix;
//layout(location = UNIFORM_MATERIAL_INDEX) uniform uint u_materialIndex;

out VS_OUT {
  vec2 texCoord;

  vec2 atlasCoord;
  flat uvec2 atlasHandle;

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

void main()
{
  const uint instanceIndex = GET_INSTANCE_INDEX;
  fillInstanceTBO(instanceIndex);

  const uint entityIndex = instance.u_entityIndex;
  fillEntityTBO(entityIndex);
  fillEntityFontTBO(entityIndex);

  #include "include/var_entity_model_matrix.glsl"

  const uint materialIndex = instance.u_materialIndex;

  vs_out.texCoord = a_texCoord;
  vs_out.materialIndex = materialIndex;

  fillMaterialTilingTBO(materialIndex);

  vs_out.texCoord.x = a_texCoord.x * material.tilingX * entity.tilingX;
  vs_out.texCoord.y = a_texCoord.y * material.tilingY * entity.tilingY;

  vs_out.atlasCoord = a_atlasCoord;
  vs_out.atlasHandle = entity.u_fontHandle;

  gl_Position = u_projectedMatrix * modelMatrix * vec4(a_pos, 1.0);
}
