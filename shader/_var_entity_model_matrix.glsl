const vec4 VEC_W = vec4(0, 0, 0, 1);

mat4 modelMatrix = transpose(mat4(
  entity.u_modelMatrixRow0,
  entity.u_modelMatrixRow1,
  entity.u_modelMatrixRow2,
  VEC_W));

if (instance.u_socketIndex >= 0) {
  modelMatrix *= u_socketTransforms[entity.u_socketBaseIndex + instance.u_socketIndex];

  // float scale = 1.0;
  // mat4 scaleMatrix = mat4(scale, 0, 0, 0,
  // 			  0, scale, 0, 0,
  // 			  0, 0, scale, 0,
  // 			  0, 0,     0, 1);

  // modelMatrix *= scaleMatrix;
} else {
  modelMatrix *= u_meshTransforms[instance.u_meshIndex];
}
