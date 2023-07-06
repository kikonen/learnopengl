vec3 calculateSpotLight(
  in SpotLight light,
  in vec3 normal,
  in vec3 toView,
  in vec3 toLight,
  in Material material)
{
  const float dist = length(toLight);

  if (dist > light.radius) return vec3(0.0);

  toLight = normalize(toLight);

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

  float attenuation = 1.0 / (light.constant + light.linear * dist +
                             light.quadratic * (dist * dist));
  diffuse  *= attenuation;
  specular *= attenuation;

  return diffuse + specular;
}
