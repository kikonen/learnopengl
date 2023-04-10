layout (std140, binding = UBO_MATRICES) uniform Matrices {
  mat4 u_projectedMatrix;
  mat4 u_projectionMatrix;
  mat4 u_viewMatrix;
  mat4 u_viewMatrixSkybox;

  mat4 u_shadowMatrix[SHADOW_MAP_COUNT];

  // NOTE KI shadow calculation only
  mat4 u_shadowProjectedMatrix[SHADOW_MAP_COUNT];
};
