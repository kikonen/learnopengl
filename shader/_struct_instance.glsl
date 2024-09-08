struct Instance {
  // vec4 u_transformRow0;
  // vec4 u_transformRow1;
  // vec4 u_transformRow2;
  mat4x3 u_transform;

  uint u_entityIndex;
  uint u_materialIndex;
  int u_socketIndex;
  uint u_flags;
};
