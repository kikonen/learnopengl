#define _UBO_SHADOW
layout(std140, binding = UBO_SHADOW) uniform Shadow {
  mat4 u_shadowMatrix[MAX_SHADOW_MAP_COUNT_ABS];

  int u_shadowCount;

  float u_shadowCascade_0;
  float u_shadowCascade_1;
  float u_shadowCascade_2;
  float u_shadowCascade_3;

  int shadow_pad1;
  int shadow_pad2;
  int shadow_pad3;
};
