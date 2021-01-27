#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 4) in float aMaterialIndex;
layout (location = 6) in mat4 aInstanceMatrix;

#include uniform_matrices.glsl

uniform mat3 normalMat;
uniform mat4 model;
uniform bool drawInstanced;

out VS_OUT {
  flat float materialIndex;
  vec3 fragPos;
  vec3 normal;

  vec4 fragPosLightSpace;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  if (drawInstanced) {
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);
  } else {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
  }

  vs_out.materialIndex = aMaterialIndex;

  vs_out.fragPos = vec3(model * vec4(aPos, 1.0));
  vs_out.normal = normalMat * aNormal;

  mat4 b = {
    {0.5f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.5f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.5f, 0.0f},
    {0.5f, 0.5f, 0.5f, 1.0f},
  };

  vs_out.fragPosLightSpace = lightSpace * model * vec4(aPos, 1.0);
}
