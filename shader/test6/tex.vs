#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec3 aBitangent;
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
  vec3 fragPos;
  vec2 texCoords;

  flat float materialIndex;
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

  if (drawInstanced) {
    gl_Position = projectionMatrix * viewMatrix * aInstanceMatrix * vec4(aPos, 1.0);
  } else {
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0);
  }

  vs_out.materialIndex = aMaterialIndex;
  vs_out.texCoords = aTexCoords;

  vs_out.fragPos = (modelMatrix * vec4(aPos, 1.0)).xyz;

  if (false && drawInstanced) {
    //mat3 mat = glm::transpose(glm::inverse(glm::mat3(aInstanceMatrix)));
    //vs_out.normal = mat * aNormal;
  } else {
    vs_out.normal = normalMatrix * aNormal;
  }

  mat4 b = {
    {0.5f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.5f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.5f, 0.0f},
    {0.5f, 0.5f, 0.5f, 1.0f},
  };

  vs_out.fragPosLightSpace = b * lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);

  bool hasNormalMap = materials[matIdx].hasNormalMap;
  if (hasNormalMap) {
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    vec3 lightPos = pointLights[0].pos;

    mat3 TBN = transpose(mat3(T, B, N));
    vs_out.tangentLightPos = TBN * lightPos;
    vs_out.tangentViewPos  = TBN * viewPos;
    vs_out.tangentFragPos  = TBN * vs_out.fragPos;
  }
}
