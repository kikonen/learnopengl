mat3 viewNormalMatrix = mat3(
  entity.u_normalMatrix0.xyz,
  entity.u_normalMatrix1.xyz,
  entity.u_normalMatrix2.xyz
  );

viewNormalMatrix *= mat3(meshMatrix);

// view-space transform applied per-pass, using whichever camera UBO is bound
viewNormalMatrix = mat3(u_viewMatrix) * viewNormalMatrix;
