#include include/struct_terrain_tile.glsl

#define _SSBO_TERRAIN_TILES
layout (std430, binding = SSBO_TERRAIN_TILES) readonly buffer TerrainTileSSBO {
  TerrainTile u_terrainTiles[];
};
