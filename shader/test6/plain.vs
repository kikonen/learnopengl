#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 4) in float aMaterialIndex;
layout (location = 6) in mat4 aInstanceMatrix;

#include uniform_matrices.glsl

uniform mat3 normalMatrix;
uniform mat4 modelMatrix;
uniform bool drawInstanced;

out VS_OUT {
  flat float materialIndex;

  vec3 fragPos;
  vec3 normal;

  vec4 fragPosLightSpace;

  vec3 tangentLightPos;
  vec3 tangentViewPos;
  vec3 tangentFragPos;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  int matIdx = int(aMaterialIndex);

  mat4 vmMat;
  if (drawInstanced) {
    vmMat = viewMatrix * aInstanceMatrix;
  } else {
    vmMat = viewMatrix * modelMatrix;
  }

  gl_Position = projectionMatrix * vmMat * vec4(aPos, 1.0);

  vs_out.materialIndex = aMaterialIndex;

  vs_out.fragPos = (modelMatrix * vec4(aPos, 1.0)).xyz;

  if (drawInstanced) {
    mat3 mat = transpose(inverse(mat3(aInstanceMatrix)));
    vs_out.normal = normalize(mat * aNormal);
  } else {
    vs_out.normal = normalize(normalMatrix * aNormal);
  }

  mat4 b = {
    {0.5f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.5f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.5f, 0.0f},
    {0.5f, 0.5f, 0.5f, 1.0f},
  };

  vs_out.fragPosLightSpace = b * lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);
}
