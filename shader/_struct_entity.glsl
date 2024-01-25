// NOTE KI using vec4 split due to "alignment is biggest element size"
// => mat4 wastes lot of space (2 * vec4)
// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct Entity {
  vec4 u_modelMatrixRow0;
  vec4 u_modelMatrixRow1;
  vec4 u_modelMatrixRow2;

  vec4 u_normalMatrix0;
  vec4 u_normalMatrix1;
  vec4 u_normalMatrix2;

  // center + radius
  vec4 u_volume;

  vec4 u_worldScale;

  int u_materialIndex;
  int u_shapeIndex;
  uint u_highlightIndex;

  uint u_objectID;
  uint u_flags;

  uint u_tileX;
  uint u_tileY;

  float u_rangeYmin;
  float u_rangeYmax;
};
