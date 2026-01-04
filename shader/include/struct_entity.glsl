// NOTE KI using vec4 split due to "alignment is biggest element size"
// => mat4 wastes lot of space (2 * vec4)
// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct Entity {
  // mat4x3 u_modelMatrix;
  vec4 u_modelMatrixRow0;
  vec4 u_modelMatrixRow1;
  vec4 u_modelMatrixRow2;

  vec4 u_normalMatrix0;
  vec4 u_normalMatrix1;
  vec4 u_normalMatrix2;

  // center + radius
  vec4 u_worldVolume;

  vec4 u_worldScale;

  uvec2 u_fontHandle;

  uint u_objectID;
  uint u_flags;

  // NOTE KI via instance "m_data" now
  // uint u_tileIndex;
  // uint u_boneBaseIndex;

  // material tiling
  float tilingX;
  float tilingY;

  int pad1;
};
