vec4 calculatePointLight(
  PointLight light,
  vec3 normal,
  vec3 toView,
  vec3 fragPos,
  Material material)
{
  vec3 toLight;
  if (material.hasNormalMap) {
    toLight = normalize(-(fs_in.tangentLightPos - fs_in.tangentFragPos));
    toView = normalize(fs_in.tangentViewPos - fs_in.tangentFragPos);
  } else {
    toLight = normalize(light.pos - fragPos);
  }

  // ambient
  vec4 ambient = light.ambient * material.ambient;

  // diffuse
  float diff = max(dot(toLight, normal), 0.0);
  vec4 diffuse = light.diffuse * (diff * material.diffuse);

  // specular
  vec3 reflectDir = reflect(-toLight, normal);
  vec3 halfwayDir = normalize(toLight + toView);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  vec4 specular = light.specular * (spec * material.specular);

  float distance = length(light.pos - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;

  vec4 lighting = ambient + diffuse + specular;
  lighting.a = material.diffuse.a;

  return lighting;
}
