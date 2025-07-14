layout(std140, binding = UBO_CAMERA) uniform Camera {
  vec3 u_cameraPos;
  vec3 u_cameraFront;
  vec3 u_cameraUp;
  vec3 u_cameraRight;

  vec3 u_mainCameraPos;
  vec3 u_mainCameraFront;
  vec3 u_mainCameraUp;
  vec3 u_mainCameraRight;

  float u_nearPlane;
  float u_farPlane;

  int pad1;
  int pad2;
};
