#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 6) in mat4 aModelMatrix;

#include uniform_matrices.glsl

out vec3 fragPos;

void main() {
  gl_Position = projectedMatrix * aModelMatrix * vec4(aPos, 1.0);

  fragPos = vec3(aModelMatrix * vec4(aPos, 1.0));
}
