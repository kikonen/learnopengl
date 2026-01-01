#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_TBN
layout (location = ATTR_TANGENT) in vec3 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include include/struct_material.glsl
#include include/struct_clip_plane.glsl
#include include/struct_entity.glsl

#include include/ssbo_entities.glsl
#include include/uniform_matrices.glsl
#include include/uniform_data.glsl
#include include/ssbo_materials.glsl
#include include/uniform_clip_planes.glsl

out VS_OUT {
  flat uint entityIndex;

  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;
  vec3 viewPos;

  flat uint materialIndex;

  flat uint shadowIndex;
  vec4 shadowPos;

#ifdef USE_TBN
  vec3 tangent;
#endif
} vs_out;

//out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Entity entity;

//#include include/fn_calculate_clipping.glsl
#include include/fn_calculate_shadow_index.glsl

void main() {
  entity = u_entities[gl_BaseInstance + gl_InstanceID];
  #include include/var_entity_model_matrix.glsl
  #include include/var_entity_normal_matrix.glsl

  const uint materialIndex = instance.u_materialIndex;

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;

  worldPos = modelMatrix * pos;

  const vec3 viewPos = (u_viewMatrix * worldPos).xyz;
  const uint shadowIndex = calculateShadowIndex(viewPos);

//  gl_Position = u_projectedMatrix * worldPos;
  gl_Position = pos;

  vs_out.entityIndex = gl_BaseInstance + gl_InstanceID;
  vs_out.materialIndex = materialIndex;

  {
    float x = entity.u_tileX;
    float y = entity.u_tileY;
    float tilingX = u_materials[materialIndex].tilingX;
    float tilingY = u_materials[materialIndex].tilingY;
    float sizeX = 1.0 / tilingX;
    float sizeY = 1.0 / tilingY;

    float scaledX = a_texCoord.x / tilingX;
    float scaledY = a_texCoord.y / tilingY;

    vs_out.texCoord.x = sizeX * x + scaledX;
    vs_out.texCoord.y = sizeY * (tilingY - (y + 1)) + scaledY;
  }

  vs_out.worldPos = worldPos.xyz;
  vs_out.vertexPos = a_pos;
  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * DECODE_A_NORMAL(a_normal);

//  calculateClipping(worldPos);

  vs_out.shadowIndex = shadowIndex;
  vs_out.shadowPos = u_shadowMatrix[shadowIndex] * worldPos;

#ifdef USE_NORMAL_TEX
  if (u_materials[materialIndex].normalMapTex.x > 0) {
    const vec3 N = normalize(vs_out.normal);
    vec3 T = normalize((normalMatrix, a_tangent);

    // NOTE KI Gram-Schmidt process to re-orthogonalize
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    T = normalize(T - dot(T, N) * N);

    //const vec3 B = cross(N, T);

    vs_out.tangent = T;
  } else {
    vs_out.tangent = DECODE_A_TANGENT(a_tangent);
  }
#endif
}
