#version 460 core

#include texture_quad.glsl

#include struct_material.glsl
#include struct_resolved_material.glsl
#include struct_decal.glsl

#include struct_clip_plane.glsl

#include ssbo_decals.glsl
#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_clip_planes.glsl
#include uniform_debug.glsl

out VS_OUT {
  vec3 worldPos;
  flat vec2 spriteCoord;
  flat vec2 spriteSize;

  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  flat uint materialIndex;

  flat uint shadowIndex;
  vec4 shadowPos;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif
} vs_out;

out float gl_ClipDistance[CLIP_COUNT];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

const vec3 UP = vec3(0, 1, 0);

Decal decal;

#include fn_calculate_clipping.glsl
#include fn_calculate_shadow_index.glsl

void main() {
  const uint decalIndex = gl_BaseInstance + gl_InstanceID;
  decal = u_decals[decalIndex];

  const vec4 VEC_W = vec4(0, 0, 0, 1);
  const mat4 modelMatrix = transpose(
    mat4(
      decal.u_transformRow0,
      decal.u_transformRow1,
      decal.u_transformRow2,
      VEC_W));
  // const mat4 modelMatrix = mat4(decal.u_transform);
  const mat3 normalMatrix = mat3(modelMatrix);

  const uint materialIndex = decal.u_materialIndex;

  vec4 pos = vec4(VERTEX_POS[gl_VertexID], 1.0);

  vec4 worldPos = modelMatrix * pos;
  vec3 normal = normalize(normalMatrix * VERTEX_NORMAL);
#ifdef USE_TBN
  vec3 tangent = normalize(normalMatrix * VERTEX_TANGENT);
#endif

  const vec3 viewPos = (u_viewMatrix * worldPos).xyz;
  const uint shadowIndex = calculateShadowIndex(viewPos);

  gl_Position = u_projectedMatrix * worldPos;

  vs_out.materialIndex = materialIndex;

  vs_out.texCoord.x = VERTEX_TEX_COORD[gl_VertexID].x * u_materials[materialIndex].tilingX;
  vs_out.texCoord.y = VERTEX_TEX_COORD[gl_VertexID].y * u_materials[materialIndex].tilingY;

  {
    const uint spriteIndex = decal.u_spriteIndex;

    const uint spritesX = u_materials[materialIndex].spritesX;
    const uint spritesY = u_materials[materialIndex].spritesY;

    const float tx = 1.0 / spritesX;
    const float ty = 1.0 / spritesY;

    const uint sx = spriteIndex % spritesX;
    const uint sy = spriteIndex / spritesX;

    vs_out.spriteCoord.x = sx * tx;
    vs_out.spriteCoord.y = 1.0 - (sy + 1) * ty;

    vs_out.spriteSize.x = tx;
    vs_out.spriteSize.y = ty;

    vs_out.texCoord.x = vs_out.spriteCoord.x + vs_out.texCoord.x * vs_out.spriteSize.x;
    vs_out.texCoord.y = vs_out.spriteCoord.y + vs_out.texCoord.y * vs_out.spriteSize.y;
  }

  vs_out.worldPos = worldPos.xyz;
  vs_out.viewPos = viewPos;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normal;

  calculateClipping(worldPos);

  vs_out.shadowIndex = shadowIndex;
  vs_out.shadowPos = u_shadowMatrix[shadowIndex] * worldPos;

#ifdef USE_TBN
  if (u_materials[materialIndex].normalMapTex.x > 0 || u_materials[materialIndex].parallaxDepth > 0)
  {
    // NOTE KI Gram-Schmidt process to re-orthogonalize
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    //tangent = normalize(tangent - dot(tangent, normal) * normal);

    const vec3 bitangent = cross(normal, tangent);

    vs_out.tbn = mat3(tangent, bitangent, normal);

#ifdef USE_PARALLAX
    const mat3 invTBN = transpose(vs_out.tbn);
    vs_out.viewTangentPos  = invTBN * u_cameraPos;
    vs_out.tangentPos  = invTBN * worldPos.xyz;
#endif
  }
#endif
}
