struct Lod {
  uint u_indexCount;  // vertexCount
                      // instanceCount == 1
  uint u_baseIndex;   // firstVertex
  uint u_baseVertex;  // baseVertex_or_baseInstance

  float u_minDistance2;
  float u_maxDistance2;

  int pad2_1;
  int pad2_2;
  int pad2_3;
};
