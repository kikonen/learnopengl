layout(std140) uniform Lights {
  DirLight u_light;
  PointLight u_pointLights[LIGHT_COUNT];
  SpotLight u_spotLights[LIGHT_COUNT];
};
