layout (std430, binding = SSBO_TERRAIN_TILES) readonly buffer TerrainTileSSBO {
  TerrainTile u_terrainTiles[];
};
