#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 4) in int aMaterialIndex;
layout (location = 5) in vec2 aTexCoords;
layout (location = 6) in mat4 aModelMatrix;
layout (location = 10) in mat3 aNormalMatrix;

#include struct_material.glsl
#include struct_texture.glsl

#include uniform_matrices.glsl
#include uniform_materials.glsl
#include uniform_data.glsl


out VS_OUT {
  vec4 glp;

  vec3 fragPos;
  vec3 normal;
  vec2 texCoords;
  vec3 vertexPos;
  vec3 viewVertexPos;

  flat int materialIndex;

  vec4 fragPosLightSpace;

  mat3 TBN;
} vs_out;


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

const mat4 b = {
  {0.5f, 0.0f, 0.0f, 0.0f},
  {0.0f, 0.5f, 0.0f, 0.0f},
  {0.0f, 0.0f, 0.5f, 0.0f},
  {0.5f, 0.5f, 0.5f, 1.0f},
};

void main() {
  vs_out.glp = projectedMatrix * aModelMatrix * vec4(aPos, 1.0);
  gl_Position = vs_out.glp;

  vs_out.materialIndex = aMaterialIndex;
  vs_out.texCoords = aTexCoords * materials[aMaterialIndex].tiling;

  vs_out.fragPos = (aModelMatrix * vec4(aPos, 1.0)).xyz;
  vs_out.vertexPos = aPos;
  vs_out.viewVertexPos = (viewMatrix * aModelMatrix * vec4(aPos, 1.0)).xyz;

  vs_out.normal = normalize(aNormalMatrix * aNormal);

  vs_out.fragPosLightSpace = b * lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);

  if (materials[aMaterialIndex].normalMapTex >= 0) {
    vec3 N = vs_out.normal;
    vec3 T = normalize(aNormalMatrix * aTangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
  }
}
