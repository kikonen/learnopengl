layout(std140, binding = UBO_DATA) uniform Data {
  vec3 u_viewWorldPos;
  vec3 u_viewFront;
  vec3 u_viewUp;
  vec3 u_viewRight;

  vec4 u_fogColor;

  vec2 u_screenResolution;

  bool u_cubeMapExist;
  bool u_frustumVisual;

  float u_fogStart;
  float u_fogEnd;
  float u_fogDensity;
  float u_time;

  float u_effectBloomExposure;
  int u_shadowCount;

  vec4 u_shadowPlanes[MAX_SHADOW_MAP_COUNT + 1];
};
