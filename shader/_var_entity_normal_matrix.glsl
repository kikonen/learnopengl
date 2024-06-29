mat3 normalMatrix = mat3(
  entity.u_normalMatrix0.xyz,
  entity.u_normalMatrix1.xyz,
  entity.u_normalMatrix2.xyz
  );

if (instance.u_socketIndex >= 0) {
  normalMatrix *= mat3(u_socketTransforms[entity.u_socketBaseIndex + instance.u_socketIndex]);
} else {
  normalMatrix *= mat3(u_meshTransforms[instance.u_meshIndex]);
}
