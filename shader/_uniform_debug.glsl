layout(std140, binding = UBO_DEBUG) uniform DebugUBO {
  int u_entityId;
  int u_boneIndex;

  bool u_boneWeight;

  float u_parallaxDepth;
  int u_parallaxMethod;

  float u_wireframeLineWidth;
  vec3 u_wireframeLineColor;
} Debug;
