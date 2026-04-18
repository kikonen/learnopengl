#ifdef USE_TBN
  // Reconstruct tangent basis in FS against the interpolated normal.
  // fs_in.tangent: xyz = tangent, w = handedness
  const vec3 N = normalize(normal);
  vec3 T = normalize(fs_in.tangent.xyz);
  // Gram-Schmidt re-orthogonalize against interpolated N
  T = normalize(T - dot(T, N) * N);
  const vec3 B = cross(N, T) * fs_in.tangent.w;
  const mat3 tbn = mat3(T, B, N);

#ifdef USE_PARALLAX
  // View-space fragment position expressed in tangent space.
  // Equivalent to transpose(tbn) * fs_in.viewPos.
  const vec3 tangentPos = vec3(
    dot(T, fs_in.viewPos),
    dot(B, fs_in.viewPos),
    dot(N, fs_in.viewPos));
#endif
#endif
