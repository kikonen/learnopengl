layout(std140, binding = UBO_DATA) uniform Data {
  vec3 u_cameraPos;
  vec3 u_cameraFront;
  vec3 u_cameraUp;
  vec3 u_cameraRight;

  vec3 u_mainCameraPos;
  vec3 u_mainCameraFront;
  vec3 u_mainCameraUp;
  vec3 u_mainCameraRight;

  vec4 u_fogColor;

  vec2 u_screenResolution;

  bool u_cubeMapExist;
  bool u_skyboxExist;

  bool u_environmentMapExist;

  bool u_shadowVisual;

  float u_fogStart;
  float u_fogEnd;
  float u_fogDensity;

  float u_hdrGamma;
  float u_hdrExposure;
  float u_effectBloomExposure;

  float u_time;
  int u_frame;
  int u_shadowCount;

  float u_shadowCascade_0;
  float u_shadowCascade_1;
  float u_shadowCascade_2;
  float u_shadowCascade_3;
};
