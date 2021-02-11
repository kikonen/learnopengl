#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aMaterialIndex;
layout (location = 3) in vec2 aTexCoords;
layout (location = 4) in vec3 aNormal;
layout (location = 6) in mat4 aInstanceMatrix;

#include uniform_matrices.glsl

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform bool drawInstanced;

out vec4 color;
flat out int materialIndex;
out vec2 texCoords;
out vec3 fragPos;
out vec3 normal;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  mat4 vmMat;
  if (drawInstanced) {
    vmMat = viewMatrix * aInstanceMatrix;
  } else {
    vmMat = viewMatrix * modelMatrix;
  }

  gl_Position = projectionMatrix * vmMat * vec4(aPos, 1.0);

  color = aColor;

  materialIndex = int(aMaterialIndex);
  texCoords = aTexCoords;

  fragPos = vec3(modelMatrix * vec4(aPos, 1.0));
  normal = normalMatrix * aNormal;
}
