// NOTE KI using vec4 split due to "alignment is biggest element size"
// => mat4 wastes lot of space (2 * vec4)
// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct Entity {
  vec4 modelMatrix0;
  vec4 modelMatrix1;
  vec4 modelMatrix2;
  vec4 modelMatrix3;

  vec4 normalMatrix0;
  vec4 normalMatrix1;
  vec4 normalMatrix2;

  // center + radius
  vec4 volume;

  vec4 worldScale;

  int materialIndex;
  int shapeIndex;
  uint highlightIndex;

  uint objectID;
  uint flags;

  uint tileX;
  uint tileY;

  float rangeYmin;
  float rangeYmax;
};
