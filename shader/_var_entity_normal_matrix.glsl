mat3 normalMatrix = mat3(
  entity.u_normalMatrix0.xyz,
  entity.u_normalMatrix1.xyz,
  entity.u_normalMatrix2.xyz
  );

#ifdef USE_SOCKETS
if (instance.u_socketIndex >= 0) {
  normalMatrix =
    normalMatrix
    * mat3(resolveSocketMatrix(entity.u_socketBaseIndex + instance.u_socketIndex + u_socketBaseIndex))
    * mat3(meshMatrix);

} else {
  normalMatrix *= mat3(meshMatrix);
}
#else
  normalMatrix *= mat3(meshMatrix);
#endif
