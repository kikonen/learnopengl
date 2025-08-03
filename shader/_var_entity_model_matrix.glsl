const vec4 VEC_W = vec4(0, 0, 0, 1);

mat4 modelMatrix = transpose(mat4(
  entity.u_modelMatrixRow0,
  entity.u_modelMatrixRow1,
  entity.u_modelMatrixRow2,
  VEC_W));

const mat4 meshMatrix = transpose(mat4(
  instance.u_transformRow0,
  instance.u_transformRow1,
  instance.u_transformRow2,
  VEC_W));

// mat4 modelMatrix = mat4(entity.u_modelMatrix);
// const mat4 meshMatrix = mat4(instance.u_transform);

#ifdef USE_SOCKETS
if (instance.u_socketIndex > 0) {
  modelMatrix =
    modelMatrix
    * resolveSocketMatrix(instance.u_socketIndex)
    * meshMatrix;
} else {
  modelMatrix *= meshMatrix;
}
#else
  modelMatrix *= meshMatrix;
#endif
