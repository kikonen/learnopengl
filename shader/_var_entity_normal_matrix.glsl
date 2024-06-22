const mat3 normalMatrix = mat3(
  entity.u_normalMatrix0.xyz,
  entity.u_normalMatrix1.xyz,
  entity.u_normalMatrix2.xyz
  ) * mat3(u_meshTransforms[instance.u_meshIndex]);
