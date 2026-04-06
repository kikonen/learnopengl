#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_NORMAL) in vec3 a_normal;
#ifdef USE_PARALLAX
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

#ifdef USE_PARALLAX
  vec3 tangentViewPos;
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

  worldPos = modelMatrix * pos;

  normal = normalize(viewNormalMatrix * normal);
#ifdef USE_PARALLAX
  vec3 tangent = normalize(viewNormalMatrix * DECODE_A_TANGENT(a_tangent));
#endif

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

  vs_out.viewPos = (u_viewMatrix * worldPos).xyz;

  vs_out.vertexPos = a_pos;

  vs_out.normal = normal;

#ifdef USE_PARALLAX
  {
    // NOTE KI Gram-Schmidt process to re-orthogonalize
    // https://learnopengl.com/Advanced-Lighting/Normal-Mapping
    tangent = normalize(tangent - dot(tangent, normal) * normal);

    const vec3 bitangent = cross(normal, tangent);
    const mat3 tbn = mat3(tangent, bitangent, normal);
    const mat3 invTBN = transpose(tbn);
    vs_out.tangentViewPos = invTBN * vs_out.viewPos.xyz;
  }
#endif
}
