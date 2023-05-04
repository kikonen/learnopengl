layout(std140, binding = UBO_DATA) uniform Data {
  vec3 u_viewWorldPos;
  vec3 u_viewFront;
  vec3 u_viewUp;
  vec3 u_viewRight;

  vec2 u_resolution;
  bool u_cubeMapExist;
  bool u_frustumVisual;

  vec4 u_fogColor;

  float u_fogStart;
  float u_fogEnd;
  float u_fogDensity;
  float u_time;

  int u_shadowCount;

  vec4 u_shadowPlanes[MAX_SHADOW_MAP_COUNT + 1];
};
