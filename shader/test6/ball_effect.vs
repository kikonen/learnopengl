#version 450 core

#include constants.glsl

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

out VS_OUT {
  vec3 fragPos;
  vec3 normal;
  vec2 texCoords;

  flat int materialIndex;

  vec4 fragPosLightSpace;
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
  gl_Position = projectedMatrix * aModelMatrix * vec4(aPos, 1.0);

  vs_out.materialIndex = aMaterialIndex;
  vs_out.texCoords = aTexCoords * materials[aMaterialIndex].tiling;

  vs_out.fragPos = (aModelMatrix * vec4(aPos, 1.0)).xyz;
  vs_out.normal = normalize(aNormalMatrix * aNormal);

  vs_out.fragPosLightSpace = b * lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);
}
