void calculateClipping(vec4 worldPos) {
  for (int i = 0; i < CLIP_COUNT; i++) {
    gl_ClipDistance[i] = clipping[i].enabled
      ? dot(worldPos.xyz, clipping[i].plane.xyz) + clipping[i].plane.w
      : 1.0;
  }
}
