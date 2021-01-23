#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 6) in mat4 aInstanceMatrix;

#include uniform_matrices.glsl

uniform mat4 model;
uniform bool drawInstanced;

out vec3 fragPos;

void main() {
  if (drawInstanced) {
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);
  } else {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
  }

  fragPos = vec3(model * vec4(aPos, 1.0));
}
