void calculateClipping(in vec4 worldPos) {
  for (int i = 0; i < u_clipCount; i++) {
    gl_ClipDistance[i] = dot(worldPos.xyz, u_clipping[i].plane.xyz)
      + u_clipping[i].plane.w;
  }
}
