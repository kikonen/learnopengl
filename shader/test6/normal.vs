#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 6) in mat4 aInstanceMatrix;

#include uniform_matrices.glsl

uniform mat4 modelMatrix;
uniform bool drawInstanced;

out VS_OUT {
  vec3 normal;
} vs_out;

void main() {
  mat4 vmMat;
  if (drawInstanced) {
    vmMat = viewMatrix * aInstanceMatrix;
  } else {
    vmMat = viewMatrix * modelMatrix;
  }

  gl_Position = vmMat * vec4(aPos, 1.0);

  mat3 normalMatrix = mat3(transpose(inverse(vmMat)));
  vs_out.normal = normalize(normalMatrix * aNormal);
}
