const vec4 VEC_W = vec4(0, 0, 0, 1);
const mat4 modelMatrix = transpose(mat4(
  entity.u_modelMatrixRow0,
  entity.u_modelMatrixRow1,
  entity.u_modelMatrixRow2,
  VEC_W));
