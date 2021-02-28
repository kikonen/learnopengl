bool calculateClipping(vec4 worldPos) {
  bool clipped = false;
  for (int i = 0; i < CLIP_COUNT; i++) {
    if (!clipping[i].enabled) {
      gl_ClipDistance[i] = -1;
      continue;
    }
    gl_ClipDistance[i] = dot(worldPos, clipping[i].plane);
    if (gl_ClipDistance[i] < 0) {
      clipped = true;
    }
  }

  return clipped;
}
