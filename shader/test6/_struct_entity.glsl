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
