struct Entity {
  mat4 modelMatrix;
  mat4 normalMatrix;

  // center + radius
  vec4 volume;

  int materialIndex;
  uint highlightIndex;

  uint objectID;
  uint flags;

  vec4 pad1;
  vec4 pad2;
};
