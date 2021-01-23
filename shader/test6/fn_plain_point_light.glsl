vec4 calculatePointLight(
  PointLight light,
  vec3 normal,
  vec3 viewDir,
  vec3 fragPos,
  vec4 matAmbient,
  vec4 matDiffuse,
  vec4 matSpecular,
  float matShininess) {
  vec3 lightDir = normalize(light.pos - fragPos);

  // ambient
  vec4 ambient = light.ambient * matAmbient;

  // diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  vec4 diffuse = light.diffuse * (diff * matDiffuse);

  // specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), matShininess);
  vec4 specular = light.specular * (spec * matSpecular);

  float distance = length(light.pos - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;

  // combined
  return ambient + diffuse + specular;
}
