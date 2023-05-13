vec3 calculateSpotLight(
  in SpotLight light,
  in vec3 normal,
  in vec3 toView,
  in vec3 worldPos,
  in Material material)
{
  vec3 toLight = normalize(light.worldPos - worldPos);

  float theta = dot(toLight, normalize(-light.worldDir));
  bool shade = theta > light.cutoff;

  vec3 diffuse = vec3(0);
  vec3 specular = vec3(0);

  if (shade) {
    float epsilon = light.cutoff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);

    // diffuse
    float diff = max(dot(normal, toLight), 0.0);
    diffuse = light.diffuse * (diff * material.diffuse.xyz);

    // specular
    const float shininess = material.specular.a;
    if (shininess > 0) {
      vec3 reflectDir = reflect(-toLight, normal);
      float spec = pow(max(dot(toView, reflectDir), 0.0), shininess);
      specular = spec * light.specular * material.specular.xyz;
    }

    diffuse  *= intensity;
    specular *= intensity;
  }

  float distance = length(light.worldPos - worldPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
  diffuse  *= attenuation;
  specular *= attenuation;

  return diffuse + specular;
}
