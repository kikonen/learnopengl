#version 460 core

#include constants.glsl

layout (location = ATTR_POS) in vec4 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_NORMAL_TEX
layout (location = ATTR_TANGENT) in vec3 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include struct_material.glsl
#include struct_clip_plane.glsl
#include struct_entity.glsl

#include uniform_entities.glsl
#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl
#include uniform_clip_planes.glsl
#include uniform_material_indeces.glsl

out VS_OUT {
  vec3 fragPos;
  vec3 normal;
  vec2 texCoord;
  vec4 vertexPos;
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

precision mediump float;

#include fn_calculate_clipping.glsl

void main() {
  Entity entity = u_entities[int(gl_BaseInstance) + gl_InstanceID];
  mat3 normalMatrix = mat3(entity.normalMatrix);
  int materialIndex = int(entity.materialIndex);
  vec4 worldPos = entity.modelMatrix * a_pos;

  if (materialIndex < 0) {
    materialIndex = int(u_materialIndeces[-materialIndex + gl_VertexID - gl_BaseVertex]);
  }

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.materialIndex = materialIndex;

  vs_out.texCoord = a_texCoord * u_materials[materialIndex].tiling;

  vs_out.fragPos = worldPos.xyz;
  vs_out.vertexPos = a_pos;
  vs_out.viewVertexPos = (u_viewMatrix * worldPos).xyz;

  vs_out.normal = normalize(normalMatrix * a_normal);

  calculateClipping(worldPos);

  vs_out.fragPosLightSpace = u_shadowMatrix * worldPos;

#ifdef USE_NORMAL_TEX
  if (u_materials[materialIndex].normalMapTex >= 0)
  {
    vec3 N = vs_out.normal;
    vec3 T = normalize(normalMatrix * a_tangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
  }
#endif
}
