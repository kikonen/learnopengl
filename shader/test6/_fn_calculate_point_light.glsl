vec4 calculatePointLight(
  in PointLight light,
  in vec3 normal,
  in vec3 toView,
  in vec3 worldPos,
  in Material material)
{
  vec3 toLight = normalize(light.worldPos - worldPos);

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

  float distance = length(light.worldPos - worldPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;

  vec4 lighting = ambient + diffuse + specular;
  lighting.a = material.diffuse.a;

  return lighting;
}
