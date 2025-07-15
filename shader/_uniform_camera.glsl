layout(std140, binding = UBO_CAMERA) uniform Camera {
  mat4 u_mainProjectedMatrix;

  mat4 u_projectedMatrix;

  mat4 u_projectionMatrix;
  mat4 u_invProjectionMatrix;

  mat4 u_viewMatrix;
  mat4 u_invViewMatrix;

  mat4 u_viewMatrixSkybox;

  mat4 u_viewportMatrix;

  mat4 u_shadowMatrix[MAX_SHADOW_MAP_COUNT_ABS];

  // top, bottom, left, right, near, far
  vec4 u_frustum[6];

  vec4 u_cameraPos;
  vec4 u_cameraFront;
  vec4 u_cameraUp;
  vec4 u_cameraRight;

  vec4 u_mainCameraPos;
  vec4 u_mainCameraFront;
  vec4 u_mainCameraUp;
  vec4 u_mainCameraRight;

  float u_nearPlane;
  float u_farPlane;

  bool u_cameraSsaoEnabled;

  int pad1;
  // int pad2;
};
