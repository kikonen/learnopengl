// NOTE KI *Too* big (like 32) array *will* cause shader to crash mysteriously
#define LIGHT_COUNT 8

layout(std140) uniform Lights {
  DirLight light;
  PointLight pointLights[LIGHT_COUNT];
  SpotLight spotLights[LIGHT_COUNT];
};
