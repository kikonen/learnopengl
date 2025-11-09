mat3 normalMatrix = mat3(
  entity.u_normalMatrix0.xyz,
  entity.u_normalMatrix1.xyz,
  entity.u_normalMatrix2.xyz
  );

normalMatrix *= mat3(meshMatrix);

// #ifdef USE_SOCKETS
// if (instance.u_socketIndex > 0) {
//   normalMatrix =
//     normalMatrix
//     * mat3(resolveSocketMatrix(instance.u_socketIndex))
//     * mat3(meshMatrix);

// } else {
//   normalMatrix *= mat3(meshMatrix);
// }
// #else
//   normalMatrix *= mat3(meshMatrix);
// #endif
