layout (std140, binding = 2) uniform ClipPlanes {
  ClipPlane u_clipping[CLIP_COUNT];
  int u_clipCount;
};
