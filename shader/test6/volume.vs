#version 460 core

#include constants.glsl

layout (location = ATTR_POS) in vec4 a_pos;
//layout (location = ATTR_INSTANCE_MODEL_MATRIX_1) in mat4 a_modelMatrix;
//layout (location = ATTR_INSTANCE_NORMAL_MATRIX_1) in mat3 a_normalMatrix;
layout (location = ATTR_INSTANCE_ENTITY_INDEX) in float a_entityIndex;

#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl

out VS_OUT {
  vec3 fragPos;
  flat uint materialIndex;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  Entity entity = u_entities[int(a_entityIndex)];
  int materialIndex = int(entity.materialIndex);
  vec4 worldPos = entity.modelMatrix * a_pos;

  gl_Position = u_projectedMatrix * worldPos;
  vs_out.fragPos = worldPos.xyz;

  vs_out.materialIndex = materialIndex;
}
