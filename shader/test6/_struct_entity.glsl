struct Entity {
  mat4 modelMatrix;
  //mat4 normalMatrix;
  vec4 objectID;

  vec3 volumeCenter;
  float volumeRadius;

  int materialIndex;
  uint highlightIndex;
  uint flags;

  uint drawType;
};
