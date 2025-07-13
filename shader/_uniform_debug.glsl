layout(std140, binding = UBO_DEBUG) uniform DebugUBO {
  vec4 u_wireframeLineColor;
  vec4 u_skyboxColor;
  vec4 u_ssaoBaseColor;

  bool u_wireframeOnly;
  float u_wireframeLineWidth;

  int u_entityId;
  int u_boneIndex;

  bool u_boneWeight;

  bool u_lightEnabled;
  bool u_normalMapEnabled;

  bool u_skyboxColorEnabled;

  bool u_ssaoEnabled;
  bool u_ssaoBaseColorEnabled;

  float u_parallaxDepth;
  int u_parallaxMethod;

  // int pad1;
  // int pad2;
  // int pad3;
} Debug;
