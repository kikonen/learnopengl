vec3 calculatePointLight(
  in PointLight light,
  in vec3 normal,
  in vec3 toView,
  in vec3 toLight,
  in Material material)
{
  const float dist = length(toLight);

  if (dist > light.radius) return vec3(0.0);

  toLight = normalize(toLight);

  const float powerup = 1.7;

  // diffuse
  float diff = max(dot(toLight, normal), 0.0);
  vec3 diffuse = powerup * light.diffuse * (diff * material.diffuse.xyz);

  // specular
  vec3 specular = vec3(0);
  const float shininess = material.specular.a;
  if (shininess > 0) {
    vec3 reflectDir = reflect(-toLight, normal);
    vec3 halfwayDir = normalize(toLight + toView);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    specular = powerup * light.specular * (spec * material.specular.xyz);
  }

  float attenuation = 1.0 / (light.constant + light.linear * dist +
                             light.quadratic * (dist * dist));
  diffuse  *= attenuation;
  specular *= attenuation;

  return diffuse + specular;
}
