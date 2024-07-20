layout(std140, binding = UBO_DEBUG) uniform Debug {
  int u_debugEntityId;
  int u_debugBoneIndex;

  bool u_debugBoneWeight;

  float u_parallaxDepth;
  int u_parallaxMethod;
};
