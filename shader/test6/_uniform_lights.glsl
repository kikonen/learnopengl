layout(std140) uniform Lights {
  DirLight light;
  PointLight pointLights[LIGHT_COUNT];
  SpotLight spotLights[LIGHT_COUNT];
};
