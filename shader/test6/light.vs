#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aMaterialIndex;
layout (location = 3) in vec2 aTexCoords;
layout (location = 4) in vec3 aNormal;
layout (location = 6) in mat4 aModelMatrix;
layout (location = 10) in mat3 aNormalMatrix;

#include uniform_matrices.glsl

out vec4 color;
flat out int materialIndex;
out vec2 texCoords;
out vec3 fragPos;
out vec3 normal;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  gl_Position = projectionMatrix * viewMatrix * aModelMatrix * vec4(aPos, 1.0);

  color = aColor;

  materialIndex = int(aMaterialIndex);
  texCoords = aTexCoords;

  fragPos = vec3(aModelMatrix * vec4(aPos, 1.0));
  normal = normalize(aNormalMatrix * aNormal);
}
