#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

#include uniform_matrices.glsl

uniform mat4 model;

out VS_OUT {
  vec3 normal;
} vs_out;

void main() {
  gl_Position = view * model * vec4(aPos, 1.0);

  mat3 normalMat = mat3(transpose(inverse(view * model)));

  vs_out.normal = normalize(normalMat * aNormal);
}
