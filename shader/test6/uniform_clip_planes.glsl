#define CLIP_COUNT 2

layout (std140) uniform ClipPlanes {
  ClipPlane clipping[CLIP_COUNT];
};
