#version 450 core

#include constants.glsl

layout (location = 4) in uint a_materialIndex;
layout (location = 6) in mat4 a_modelMatrix;
layout (location = 10) in mat3 a_normalMatrix;

#include struct_clip_plane.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_clip_planes.glsl

out VS_OUT {
  vec3 fragPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewVertexPos;

  flat uint materialIndex;

  vec4 fragPosLightSpace;

#ifdef USE_NORMAL_TEX
  flat mat3 TBN;
#endif
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_calculate_clipping.glsl

void main() {
  vec2 texCoord = vec2(0.0, 0.0);
  vec3 pos = vec3(0.0, 0.0, 0.0);
  vec3 normal = vec3(0.0, 0.0, 1.0);

  vec4 worldPos = a_modelMatrix * vec4(pos, 1.0);

  gl_Position =  worldPos;

  vs_out.materialIndex = a_materialIndex;

  vs_out.texCoord = texCoord;

  vs_out.fragPos = worldPos.xyz;
  vs_out.vertexPos = pos;
  vs_out.viewVertexPos = (u_viewMatrix * worldPos).xyz;

  vs_out.normal = normalize(a_normalMatrix * normal);

  calculateClipping(worldPos);

  vs_out.fragPosLightSpace = u_shadowMatrix * worldPos;

#ifdef USE_NORMAL_TEX
  {
    vec3 tangent = vec4(1.0, 0.0, 0.0);

    vec3 N = vs_out.normal;
    vec3 T = normalize(a_normalMatrix * tangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
  }
#endif
}
