#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 6) in mat4 aInstanceMatrix;
layout (location = 10) in mat3 aNormalMatrix;

#include uniform_matrices.glsl


out VS_OUT {
  vec3 normal;
} vs_out;

void main() {
  mat4 vmMat = viewMatrix * aModelMatrix;

  gl_Position = vmMat * vec4(aPos, 1.0);

  mat3 normalMatrix = mat3(transpose(inverse(vmMat)));
  vs_out.normal = normalize(normalMatrix * aNormal);
}
