#version 450 core

#include constants.glsl

layout (location = ATTR_MATERIAL_INDEX) in uint a_materialIndex;
layout (location = ATTR_INSTANCE_MODEL_MATRIX_1) in mat4 a_modelMatrix;
layout (location = ATTR_INSTANCE_NORMAL_MATRIX_1) in mat3 a_normalMatrix;

#include uniform_matrices.glsl
#include uniform_data.glsl

out VS_OUT {
  flat uint materialIndex;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  vec4 worldPos = a_modelMatrix * vec4(0.0, 0.0, 0.0, 1.0);

  vec4 pos = u_projectedMatrix * worldPos;

  vs_out.materialIndex = a_materialIndex;
  gl_PointSize = 64;//(1.0 - pos.z / pos.w) * 64.0;
  gl_Position = pos;
}
