layout (std140, binding = UBO_CLIP_PLANES) uniform ClipPlanes {
  int u_clipCount;
  ClipPlane u_clipping[CLIP_COUNT];
};
