#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_TBN
layout (location = ATTR_TANGENT) in vec3 a_tangent;
#endif
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include "include/ssbo_entities.glsl"
#include "include/ssbo_instances.glsl"
#include "include/ssbo_instance_indeces.glsl"
#include "include/ssbo_socket_transforms.glsl"
#include "include/ssbo_materials.glsl"
#include "include/ssbo_terrain_tiles.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"

out VS_OUT {
  flat uint entityIndex;
  flat uint instanceIndex;

#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;
  vec3 vertexPos;

  flat uint materialIndex;

  flat uint tileIndex;
  flat uint tileX;
  flat uint tileY;

  flat float tilingX;
  flat float rangeYmin;
  flat float rangeYmax;
  flat uvec2 heightMapTex;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  flat vec3 viewTangentPos;
  vec3 tangentPos;
#endif
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Instance instance;
Entity entity;

ResolvedMaterial material;

TerrainTile tile;

void main() {
  const uint instanceIndex = GET_INSTANCE_INDEX;
  instance = u_instances[instanceIndex];

  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  const uint tileIndex = instance.u_data;
  tile = u_terrainTiles[tileIndex];

  #include "include/var_entity_model_matrix.glsl"
  #include "include/var_entity_normal_matrix.glsl"

  const uint materialIndex = instance.u_materialIndex;

  const vec4 pos = vec4(a_pos, 1.0);
  vec4 worldPos;
  vec3 normal = DECODE_A_NORMAL(a_normal);
#ifdef USE_TBN
  vec3 tangent = DECODE_A_TANGENT(a_tangent);
#endif

  worldPos = modelMatrix * pos;

//  gl_Position = u_projectedMatrix * worldPos;
  gl_Position = pos;

  vs_out.entityIndex = entityIndex;
  vs_out.instanceIndex = instanceIndex;
  vs_out.materialIndex = materialIndex;
  vs_out.tileIndex = tileIndex;
  vs_out.tileX = tile.u_tileX;
  vs_out.tileY = tile.u_tileY;


  vs_out.rangeYmin = tile.u_rangeYmin;
  vs_out.rangeYmax = tile.u_rangeYmax;
  vs_out.heightMapTex = tile.heightMapTex;

  {
    float x = tile.u_tileX;
    float y = tile.u_tileY;
    float tilingX = u_materials[materialIndex].tilingX * entity.tilingX;
    float tilingY = u_materials[materialIndex].tilingY * entity.tilingY;
    float sizeX = 1.0 / tilingX;
    float sizeY = 1.0 / tilingY;

    float scaledX = a_texCoord.x / tilingX;
    float scaledY = a_texCoord.y / tilingY;

    vs_out.texCoord.x = sizeX * x + scaledX;
    vs_out.texCoord.y = sizeY * (tilingY - (y + 1)) + scaledY;

    vs_out.tilingX = tilingX;
  }

#ifdef USE_CUBE_MAP
  vs_out.worldPos = worldPos.xyz;
#endif
  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  vs_out.vertexPos = a_pos;

  // NOTE KI pointless to normalize vs side
  vs_out.normal = normalMatrix * normal;

#ifdef USE_TBN
  if (u_materials[materialIndex].normalMapTex.x > 0) {
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
}
