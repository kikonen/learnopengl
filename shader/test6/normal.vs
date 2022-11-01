#version 450 core

#include constants.glsl

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 6) in mat4 a_modelMatrix;
layout (location = 10) in mat3 a_normalMatrix;

#include uniform_matrices.glsl

out VS_OUT {
  vec3 normal;
} vs_out;

void main() {
  vec4 worldPos = a_modelMatrix * vec4(a_pos, 1.0);

  gl_Position = u_viewMatrix * worldPos;

  vs_out.normal = normalize(mat3(u_viewMatrix) * a_normalMatrix * a_normal);
}
