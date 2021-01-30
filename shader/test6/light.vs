#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aMaterialIndex;
layout (location = 3) in vec2 aTexCoords;
layout (location = 4) in vec3 aNormal;

#include uniform_matrices.glsl

//uniform mat4 transform;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

out vec4 color;
flat out float materialIndex;
out vec2 texCoords;
out vec3 fragPos;
out vec3 normal;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0);

  color = aColor;

  materialIndex = aMaterialIndex;
  texCoords = aTexCoords;

  fragPos = vec3(modelMatrix * vec4(aPos, 1.0));
  normal = normalMatrix * aNormal;
}
