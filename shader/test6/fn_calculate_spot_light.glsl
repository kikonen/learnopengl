vec4 calculateSpotLight(
  SpotLight light,
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

  float theta = dot(toLight, normalize(-light.dir));
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
    vec3 reflectDir = reflect(-toLight, normal);
    float spec = pow(max(dot(toView, reflectDir), 0.0), material.shininess);
    specular = light.specular * (spec * material.specular);

    diffuse  *= intensity;
    specular *= intensity;
  } else {
    ambient = light.ambient * material.diffuse;
  }

  float distance = length(light.pos - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;

  // combined
  return ambient + diffuse + specular;
}
