struct MeshInstance {
  // mat4x3 u_transform;
  vec4 u_transformRow0;
  vec4 u_transformRow1;
  vec4 u_transformRow2;

  uint u_entityIndex;
  uint u_lodBaseIndex;
  uint u_lodCount;

  uint u_materialIndex;
  int u_socketIndex;
  uint u_flags;

  // case specific data
  // - terrain: terrain tile index
  uint u_data;

  int pad2_1;
  // int pad2_2;
  // int pad2_3;
};
