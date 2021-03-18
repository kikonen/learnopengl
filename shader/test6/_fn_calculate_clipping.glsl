void calculateClipping(vec4 worldPos) {
  for (int i = 0; i < CLIP_COUNT; i++) {
    gl_ClipDistance[i] = clipping[i].enabled ? dot(worldPos, clipping[i].plane) : 1.0;
  }
}
