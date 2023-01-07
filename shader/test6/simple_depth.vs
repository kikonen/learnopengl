#version 460 core

#include constants.glsl

layout (location = ATTR_POS) in vec4 a_pos;
#ifdef USE_ALPHA
//layout (location = ATTR_MATERIAL_INDEX) in float a_materialIndex;
layout (location = ATTR_TEX) in vec2 a_texCoord;
#endif
//layout (location = ATTR_INSTANCE_MODEL_MATRIX_1) in mat4 a_modelMatrix;
layout (location = ATTR_INSTANCE_ENTITY_INDEX) in float a_entityIndex;

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl

#ifdef USE_ALPHA
out VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
} vs_out;
#endif

void main()
{
  Entity entity = u_entities[int(a_entityIndex)];
  vec4 worldPos = entity.modelMatrix * a_pos;

  gl_Position = u_lightProjectedMatrix * worldPos;

#ifdef USE_ALPHA
  int materialIndex = int(entity.materialIndex);
  vs_out.materialIndex = materialIndex;
  vs_out.texCoord = a_texCoord;
#endif
}
