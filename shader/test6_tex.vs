#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in float aMaterialIndex;
layout (location = 3) in vec2 aTexCoords;
layout (location = 4) in mat4 aInstanceMatrix;

layout (std140) uniform Matrices {
  mat4 projection;
  mat4 view;
};

uniform mat3 normalMat;
uniform mat4 model;
uniform bool drawInstanced;

flat out float materialIndex;
out vec2 texCoords;
out vec3 fragPos;
out vec3 normal;

void main() {
  if (drawInstanced) {
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);
  } else {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
  }

  materialIndex = aMaterialIndex;
  texCoords = aTexCoords;

  fragPos = vec3(model * vec4(aPos, 1.0));
  normal = normalMat * aNormal;
}
