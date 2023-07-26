vec3 calculatePointLight(
  in PointLight light,
  in vec3 normal,
  in vec3 viewDir,
  in vec3 worldPos,
  in Material material)
{
  const vec3 toLight = light.worldPos.xyz - worldPos;
  const float dist = length(toLight);

  if (dist > light.radius) return vec3(0.0);

  const vec3 lightDir = normalize(toLight);

  const float powerup = 1.7;

  // diffuse
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = powerup * light.diffuse.rgb * (diff * material.diffuse.rgb);

  // specular
  vec3 specular = vec3(0);
  const float shininess = material.specular.a;
  if (shininess > 0) {
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    specular = powerup * light.specular.rgb * (spec * material.specular.rgb);
  }

  float attenuation = 1.0 / (light.constant + light.linear * dist +
                             light.quadratic * (dist * dist));
  diffuse  *= attenuation;
  specular *= attenuation;

  return diffuse + specular;
}
