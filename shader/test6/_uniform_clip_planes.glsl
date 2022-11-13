layout (std140, binding = UBO_CLIP_PLANES) uniform ClipPlanes {
  ClipPlane u_clipping[CLIP_COUNT];
  int u_clipCount;
};
