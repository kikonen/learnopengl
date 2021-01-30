#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

#include uniform_matrices.glsl

uniform mat4 modelMatrix;

out VS_OUT {
  vec3 normal;
} vs_out;

void main() {
  gl_Position = viewMatrix * modelMatrix * vec4(aPos, 1.0);

  mat3 normalMatrix = mat3(transpose(inverse(viewMatrix * modelMatrix)));

  vs_out.normal = normalize(normalMatrix * aNormal);
}
