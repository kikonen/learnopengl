// NOTE KI using vec4 split due to "alignment is biggest element size"
// => mat4 wastes lot of space (2 * vec4)
// NOTE KI https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
struct Decal {
  // vec4 u_transformRow0;
  // vec4 u_transformRow1;
  // vec4 u_transformRow2;
  mat4x3 u_transform;

  uint u_materialIndex;
  uint u_spriteIndex;
};
