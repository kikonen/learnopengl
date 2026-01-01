// NOTE KI using vec4 split due to "alignment is biggest element size"
// => mat4 wastes lot of space (2 * vec4)
// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct TerrainTile {
  uint u_tileX;
  uint u_tileY;

  float u_rangeYmin;
  float u_rangeYmax;

  uvec2 heightMapTex;
};
