vec4 calculatePointLight(
  PointLight light,
  vec3 normal,
  vec3 viewDir,
  vec3 fragPos,
  Material material)
{
  vec3 lightDir;
  if (material.hasNormalMap) {
    lightDir = normalize(-(fs_in.tangentLightPos - fs_in.tangentFragPos));
    viewDir = normalize(fs_in.tangentViewPos - fs_in.tangentFragPos);
  } else {
    lightDir = normalize(light.pos - fragPos);
  }

  // ambient
  vec4 ambient = light.ambient * material.ambient;

  // diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  vec4 diffuse = light.diffuse * (diff * material.diffuse);

  // specular
  vec3 reflectDir = reflect(-lightDir, normal);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
  vec4 specular = light.specular * (spec * material.specular);

  float distance = length(light.pos - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;

  // combined
  return ambient + diffuse + specular;
}
