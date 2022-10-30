void calculateClipping(vec4 worldPos) {
  for (int i = 0; i < CLIP_COUNT; i++) {
    gl_ClipDistance[i] = u_clipping[i].enabled
      ? dot(worldPos.xyz, u_clipping[i].plane.xyz) + u_clipping[i].plane.w
      : 1.0;
  }
}
