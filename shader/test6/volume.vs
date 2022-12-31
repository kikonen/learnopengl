#version 450 core

#include constants.glsl

layout (location = ATTR_POS) in vec4 a_pos;
layout (location = ATTR_INSTANCE_MODEL_MATRIX_1) in mat4 a_modelMatrix;
layout (location = ATTR_INSTANCE_MATERIAL_INDEX) in float a_materialIndex;

#include uniform_matrices.glsl

out VS_OUT {
  vec3 fragPos;
  flat uint materialIndex;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  vec4 worldPos = a_modelMatrix * a_pos;

  gl_Position = u_projectedMatrix * worldPos;
  vs_out.fragPos = worldPos.xyz;

  vs_out.materialIndex = int(a_materialIndex);
}
