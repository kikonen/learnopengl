#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 4) in float aMaterialIndex;
layout (location = 5) in vec2 aTexCoords;
layout (location = 6) in mat4 aInstanceMatrix;

#include struct_lights.glsl
#include struct_material.glsl
#include struct_texture.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl

uniform mat3 normalMatrix;
uniform mat4 modelMatrix;
uniform bool drawInstanced;

out VS_OUT {
  vec2 texCoords;
  vec3 vertexPos;

  flat int materialIndex;

  vec3 fragPos;
  vec3 normal;

  vec4 fragPosLightSpace;

  mat3 TBN;
  vec3 tangentLightPos;
  vec3 tangentViewPos;
  vec3 tangentFragPos;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  int matIdx = int(aMaterialIndex);

  mat4 modelMat;
  mat3 normalMat;

  if (drawInstanced) {
    modelMat = aInstanceMatrix;
    normalMat = transpose(inverse(mat3(modelMat)));
  } else {
    modelMat = modelMatrix;
    normalMat = normalMatrix;
  }

  gl_Position = projectionMatrix * viewMatrix * modelMat * vec4(aPos, 1.0);

  vs_out.materialIndex = int(aMaterialIndex);
  vs_out.texCoords = aTexCoords;

  vs_out.fragPos = (modelMat * vec4(aPos, 1.0)).xyz;
  vs_out.vertexPos = aPos;

  vs_out.normal = normalize(normalMat * aNormal);

  mat4 b = {
    {0.5f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.5f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.5f, 0.0f},
    {0.5f, 0.5f, 0.5f, 1.0f},
  };

  vs_out.fragPosLightSpace = b * lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);

  if (materials[matIdx].hasNormalMap) {
    vec3 N = vs_out.normal;
    vec3 T = normalize(normalMat * aTangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

//    vec3 lightPos = pointLights[0].pos;

    vs_out.TBN = mat3(T, B, N);
    // vs_out.tangentLightPos = TBN * lightPos;
    // vs_out.tangentViewPos  = TBN * viewPos;
    // vs_out.tangentFragPos  = TBN * vs_out.fragPos;
  }
}
