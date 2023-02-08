// NOTE KI using vec4 split due to "alignment is biggest element size"
// => mat4 wastes lot of space (2 * vec4)
struct Entity {
  vec4 modelMatrix0;
  vec4 modelMatrix1;
  vec4 modelMatrix2;
  vec4 modelMatrix3;

  vec3 normalMatrix0;
  vec3 normalMatrix1;
  vec3 normalMatrix2;

  // center + radius
  vec4 volume;

  int materialIndex;
  uint highlightIndex;

  uint objectID;
  uint flags;

//  vec4 pad1;
//  vec4 pad2;
};
