const vec4 VEC_W = vec4(0, 0, 0, 1);

mat4 modelMatrix = transpose(mat4(
  entity.u_modelMatrixRow0,
  entity.u_modelMatrixRow1,
  entity.u_modelMatrixRow2,
  VEC_W));

mat4 meshMatrix = transpose(mat4(
  instance.u_transformRow0,
  instance.u_transformRow1,
  instance.u_transformRow2,
  VEC_W));

if (instance.u_socketIndex >= 0) {
  // modelMatrix *= u_socketTransforms[entity.u_socketBaseIndex + instance.u_socketIndex];
  modelMatrix =
    modelMatrix
    * u_socketTransforms[entity.u_socketBaseIndex + instance.u_socketIndex]
    * meshMatrix;

  // float scale = 1.0;
  // mat4 scaleMatrix = mat4(scale, 0, 0, 0,
  // 			  0, scale, 0, 0,
  // 			  0, 0, scale, 0,
  // 			  0, 0,     0, 1);

  // modelMatrix *= scaleMatrix;
} else {
  modelMatrix *= meshMatrix;
}
