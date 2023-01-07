#version 460 core

#include constants.glsl

layout (location = ATTR_POS) in vec4 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
layout (location = ATTR_TEX) in vec2 a_texCoord;
//layout (location = ATTR_MATERIAL_INDEX) in float a_materialIndex;
//layout (location = ATTR_INSTANCE_MODEL_MATRIX_1) in mat4 a_modelMatrix;
//layout (location = ATTR_INSTANCE_NORMAL_MATRIX_1) in mat3 a_normalMatrix;
layout (location = ATTR_INSTANCE_ENTITY_INDEX) in float a_entityIndex;

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_clipping.glsl

void main() {
  Entity entity = u_entities[int(a_entityIndex)];
  mat3 normalMatrix = mat3(entity.normalMatrix);
  int materialIndex = int(entity.materialIndex);
  vec4 worldPos = entity.modelMatrix * a_pos;

  mat4 vmMat = u_viewMatrix * entity.modelMatrix;

  gl_Position = vmMat * a_pos;

  vs_out.materialIndex = materialIndex;

  vs_out.texCoord = a_texCoord * u_materials[materialIndex].tiling;

  vs_out.normal = normalize(normalMatrix * a_normal);

  calculateClipping(worldPos);
}
