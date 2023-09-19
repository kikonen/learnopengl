layout(std140, binding = UBO_DATA) uniform Data {
  vec3 u_viewWorldPos;
  vec3 u_viewFront;
  vec3 u_viewUp;
  vec3 u_viewRight;

  vec3 u_mainViewWorldPos;
  vec3 u_mainViewFront;
  vec3 u_mainViewUp;
  vec3 u_mainViewRight;

  vec4 u_fogColor;

  vec2 u_screenResolution;

  bool u_cubeMapExist;
  bool u_skyboxExist;

  bool u_environmentMapExist;

  bool u_frustumVisual;

  float u_fogStart;
  float u_fogEnd;
  float u_fogDensity;

  float u_hdrGamma;
  float u_hdrExposure;
  float u_effectBloomExposure;

  float u_time;
  int u_shadowCount;

  vec4 u_shadowPlanes[MAX_SHADOW_MAP_COUNT + 1];
};
