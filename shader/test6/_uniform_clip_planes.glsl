layout (std140) uniform ClipPlanes {
  ClipPlane u_clipping[CLIP_COUNT];
  int u_clipCount;
};
