#include "include/struct_lights.glsl"

#define _UBO_LIGHTS
layout(std140, binding = UBO_LIGHTS) uniform Lights {
  uint u_dirLightCount;
  uint u_pointLightCount;
  uint u_spotLightCount;

  DirLight u_dirLights[1];
  PointLight u_pointLights[LIGHT_COUNT];
  SpotLight u_spotLights[LIGHT_COUNT];
};
