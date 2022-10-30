#version 450 core

#include constants.glsl

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_tangent;
layout (location = 4) in int a_materialIndex;
layout (location = 5) in vec2 a_texCoords;
layout (location = 6) in mat4 a_modelMatrix;
layout (location = 10) in mat3 a_normalMatrix;

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

#ifdef USE_NORMAL_TEX
  mat3 TBN;
#endif
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
  vs_out.glp = u_projectedMatrix * a_modelMatrix * vec4(a_pos, 1.0);
  gl_Position = vs_out.glp;

  vs_out.materialIndex = a_materialIndex;
  vs_out.texCoords = a_texCoords * u_materials[a_materialIndex].tiling;

  vs_out.fragPos = (a_modelMatrix * vec4(a_pos, 1.0)).xyz;
  vs_out.vertexPos = a_pos;
  vs_out.viewVertexPos = (u_viewMatrix * a_modelMatrix * vec4(a_pos, 1.0)).xyz;

  vs_out.normal = normalize(a_normalMatrix * a_normal);

  vs_out.fragPosLightSpace = b * u_lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);

#ifdef USE_NORMAL_TEX
  if (u_materials[a_materialIndex].normalMapTex >= 0) {
    vec3 N = vs_out.normal;
    vec3 T = normalize(a_normalMatrix * a_tangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
  }
#endif
}
