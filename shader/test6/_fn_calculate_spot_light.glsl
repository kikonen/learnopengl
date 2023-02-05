vec4 calculateSpotLight(
  in SpotLight light,
  in vec3 normal,
  in vec3 toView,
  in vec3 worldPos,
  in Material material)
{
  vec3 toLight = normalize(light.worldPos - worldPos);

  float theta = dot(toLight, normalize(-light.worldDir));
  bool shade = theta > light.cutoff;

  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  if (shade) {
    float epsilon = light.cutoff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

    // ambient
    ambient = light.ambient * material.ambient;

    // diffuse
    float diff = max(dot(normal, toLight), 0.0);
    diffuse = light.diffuse * (diff * material.diffuse);

    // specular
    vec4 specular = vec4(0);
    if (material.shininess > 0) {
      vec3 reflectDir = reflect(-toLight, normal);
      float spec = pow(max(dot(toView, reflectDir), 0.0), material.shininess);
      specular = spec * light.specular * material.specular;
    }

    diffuse  *= intensity;
    specular *= intensity;
  } else {
    ambient = light.ambient * material.diffuse;
  }

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
