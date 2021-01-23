#version 330 core

// NOTE KI *Too* big (like 32) array *will* cause shader to crash mysteriously
#define MAT_COUNT 8
#define LIGHT_COUNT 8

#include struct_lights.glsl
#include struct_material.glsl
#include struct_texture.glsl

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aBitangent;
layout (location = 4) in float aMaterialIndex;
layout (location = 5) in vec2 aTexCoords;
layout (location = 6) in mat4 aInstanceMatrix;
// TODO KI InstanceNormalMatrix for lights

layout (std140) uniform Matrices {
  mat4 projection;
  mat4 view;
  mat4 lightSpace;
};

layout(std140) uniform Data {
  vec3 viewPos;
  float time;
};

layout(std140) uniform Lights {
  DirLight light;
  PointLight pointLights[LIGHT_COUNT];
  SpotLight spotLights[LIGHT_COUNT];
};

layout (std140) uniform Materials {
  Material materials[MAT_COUNT];
};

uniform mat3 normalMat;
uniform mat4 model;
uniform bool drawInstanced;

out VS_OUT {
  vec3 fragPos;
  vec2 texCoords;

  flat float materialIndex;
  vec3 normal;

  vec3 tangentLightPos;
  vec3 tangentViewPos;
  vec3 tangentFragPos;
} vs_out;


void main() {
  int matIdx = int(aMaterialIndex);

  if (drawInstanced) {
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);
  } else {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
  }

  vs_out.materialIndex = aMaterialIndex;
  vs_out.texCoords = aTexCoords;

  vs_out.fragPos = vec3(model * vec4(aPos, 1.0));
  vs_out.normal = normalMat * aNormal;


  bool hasNormalMap = materials[matIdx].hasNormalMap;
  if (hasNormalMap) {
    vec3 T = normalize(normalMat * aTangent);
    vec3 N = normalize(normalMat * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    vec3 lightPos = pointLights[0].pos;

    mat3 TBN = transpose(mat3(T, B, N));
    vs_out.tangentLightPos = TBN * lightPos;
    vs_out.tangentViewPos  = TBN * viewPos;
    vs_out.tangentFragPos  = TBN * vs_out.fragPos;
  }
}
