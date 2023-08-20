layout (std140, binding = UBO_MATRICES) uniform Matrices {
  mat4 u_mainProjectedMatrix;

  mat4 u_projectedMatrix;

  mat4 u_projectionMatrix;
  mat4 u_invProjectionMatrix;

  mat4 u_viewMatrix;
  mat4 u_invViewMatrix;

  mat4 u_viewMatrixSkybox;

  mat4 u_shadowMatrix[MAX_SHADOW_MAP_COUNT];

  // top, bottom, left, right, near, far
  vec4 u_frustum[6];
};
