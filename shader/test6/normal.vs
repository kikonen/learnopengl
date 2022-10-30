#version 450 core

#include constants.glsl

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 6) in mat4 aModelMatrix;
layout (location = 10) in mat3 aNormalMatrix;

#include uniform_matrices.glsl

out VS_OUT {
  vec3 normal;
} vs_out;

void main() {
  gl_Position = u_viewMatrix * aModelMatrix * vec4(aPos, 1.0);

  vs_out.normal = normalize(mat3(u_viewMatrix) * aNormalMatrix * aNormal);
}
