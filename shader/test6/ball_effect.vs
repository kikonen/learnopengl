#version 450 core

#include constants.glsl

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 4) in uint a_materialIndex;
layout (location = 5) in vec2 a_texCoords;
layout (location = 6) in mat4 a_modelMatrix;
layout (location = 10) in mat3 a_normalMatrix;

#include struct_material.glsl
#include struct_texture.glsl
#include uniform_matrices.glsl

#include uniform_materials.glsl

out VS_OUT {
  vec3 fragPos;
  vec3 normal;
  vec2 texCoords;

  flat uint materialIndex;

  vec4 fragPosLightSpace;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  vec4 worldPos = a_modelMatrix * vec4(a_pos, 1.0);

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.materialIndex = a_materialIndex;
  vs_out.texCoords = a_texCoords * u_materials[a_materialIndex].tiling;

  vs_out.fragPos = worldPos.xyz;
  vs_out.normal = normalize(a_normalMatrix * a_normal);

  vs_out.fragPosLightSpace = u_shadowMatrix * worldPos;
}
