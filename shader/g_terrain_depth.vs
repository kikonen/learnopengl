#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_TEX) in vec2 a_texCoord;

#include "include/ssbo_entities.glsl"
#include "include/ssbo_instance_indeces.glsl"
#include "include/ssbo_materials.glsl"
#include "include/ssbo_terrain_tiles.glsl"

#include "include/uniform_matrices.glsl"
#include "include/uniform_camera.glsl"
#include "include/uniform_data.glsl"

out VS_OUT {
  flat uint entityIndex;
  flat uint instanceIndex;

  vec2 texCoord;
  vec3 vertexPos;

  flat float tilingX;
  flat float rangeYmin;
  flat float rangeYmax;
  flat uvec2 heightMapTex;
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
  const uint instanceIndex = gl_BaseInstance + gl_InstanceID;
  instance = u_instances[instanceIndex];

  const uint entityIndex = instance.u_entityIndex;
  entity = u_entities[entityIndex];

  const uint tileIndex = instance.u_data;
  tile = u_terrainTiles[tileIndex];

  #include "include/var_entity_model_matrix.glsl"

  const uint materialIndex = instance.u_materialIndex;

  const vec4 pos = vec4(a_pos, 1.0);
  gl_Position = pos;

  vs_out.entityIndex = entityIndex;
  vs_out.instanceIndex = instanceIndex;

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

  vs_out.vertexPos = a_pos;
}
