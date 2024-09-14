struct Instance {
  // mat4x3 u_transform;
  vec4 u_transformRow0;
  vec4 u_transformRow1;
  vec4 u_transformRow2;

  uint u_entityIndex;
  uint u_materialIndex;
  int u_socketIndex;
  uint u_flags;
};
