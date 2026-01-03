#version 460 core

#define USE_BONES_NORMAL 1

#include "include/unit_cube_strip.glsl"

#include "include/ssbo_decals.glsl"
#include "include/ssbo_materials.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"
#include "include/uniform_clip_planes.glsl"
#include "include/uniform_debug.glsl"

out VS_OUT {
  mat4 worlToLocalMatrix;

#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  flat vec2 spriteCoord;
  flat vec2 spriteSize;

  vec3 viewPos;
  flat vec3 decalNormal;

  flat uint materialIndex;

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

const vec3 QUAD_NORMAL = vec3(0, 0, 1);
const vec3 QUAD_TANGENT = vec3(1, 0, 0);
const vec3 UP = vec3(0, 1, 0);

Decal decal;

#include "include/fn_calculate_clipping.glsl"

void main() {
  const uint vertexIndex = VERTEX_INDECES[gl_VertexID];
  const uint decalIndex = gl_BaseInstance + gl_InstanceID;
  decal = u_decals[decalIndex];

  const vec4 VEC_W = vec4(0, 0, 0, 1);
  const mat4 modelMatrix = transpose(
    mat4(
      decal.u_transformRow0,
      decal.u_transformRow1,
      decal.u_transformRow2,
      VEC_W));
  const mat3 normalMatrix = mat3(modelMatrix);

  const uint materialIndex = decal.u_materialIndex;

  vec4 pos = vec4(VERTEX_POS[vertexIndex], 1.0);

  vec4 worldPos = modelMatrix * pos;
  vec3 normal = normalize(normalMatrix * QUAD_NORMAL);
#ifdef USE_TBN
  vec3 tangent = normalize(normalMatrix * QUAD_TANGENT);
#endif

  gl_Position = u_projectedMatrix * worldPos;

  // TODO KI transpose == inverse here
  vs_out.worlToLocalMatrix = inverse(modelMatrix);

  vs_out.materialIndex = materialIndex;

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
  }

#ifdef USE_CUBE_MAP
  vs_out.worldPos = worldPos.xyz;
#endif

  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  // NOTE KI pointless to normalize vs side
  vs_out.decalNormal = normal;

  calculateClipping(worldPos);

  for (int i = 0; i < CLIP_COUNT; i++) {
    gl_ClipDistance[i] = -1.0;
  }

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
    vs_out.viewTangentPos  = invTBN * u_cameraPos.xyz;
    vs_out.tangentPos  = invTBN * worldPos.xyz;
#endif
  } else {
    vs_out.tbn = mat3(1);
  }
#endif

#ifdef USE_GL_POINTS
  // HACK KI for primitive GL_POINTS
  gl_PointSize = u_materials[materialIndex].layersDepth;
#endif
}
