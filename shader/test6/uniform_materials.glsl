// NOTE KI *Too* big (like 32) array *will* cause shader to crash mysteriously
#define MAT_COUNT 4
#define TEX_COUNT 8

layout (std140) uniform Materials {
  Material materials[MAT_COUNT];
};
