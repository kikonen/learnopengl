float calcShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(shadowMap, projCoords.xy).r;
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;

  // check whether current frag pos is in shadow
  float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
  float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
  return shadow;
}


/*
float calcShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
  float shadow = textureProj(shadowMap, fragPosLightSpace, 0.05);
  return shadow;
}
*/

vec4 calculateDirLight(
  DirLight light,
  vec3 normal,
  vec3 viewDir,
  vec4 matAmbient,
  vec4 matDiffuse,
  vec4 matSpecular,
  float matShininess,
  bool hasNormalMap,
  vec4 fragPosLightSpace)
{
  vec3 lightDir = normalize(-light.dir);

  // ambient
  vec4 ambient = light.ambient * matAmbient;

  // diffuse
  vec4 diffuse = max(dot(normal, lightDir), 0.0) * light.diffuse * matDiffuse;

  // specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), matShininess);
  vec4 specular = spec * light.specular * matSpecular;

  // calculate shadow
  float shadow = calcShadow(fragPosLightSpace, normal, lightDir);
  //shadow = 0;
  vec4 lighting = ambient + (1.0 - shadow) * (diffuse + specular);
  if (shadow > 0) {
    //lighting = vec4(1, 0, 0, 1);
  }

  return lighting;
  //return ambient + diffuse + specular;
}

vec4 calculateDirLight2(
  DirLight light,
  vec3 normal,
  vec3 viewDir,
  vec4 matAmbient,
  vec4 matDiffuse,
  vec4 matSpecular,
  float matShininess,
  bool hasNormalMap,
  vec4 fragPosLightSpace)
{
  // ambient
  vec4 ambient = light.ambient * matDiffuse;

  // diffuse
  vec3 lightDir = normalize(light.pos - fs_in.fragPos);
  //vec3 lightDir = normalize(-light.dir);
  float diff = max(dot(lightDir, normal), 0.0);
  vec4 diffuse = diff * light.diffuse;

  // specular
  vec3 reflectDir = reflect(-lightDir, normal);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), matShininess);
  vec4 specular = spec * light.specular;

  // calculate shadow
  float shadow = calcShadow(fs_in.fragPosLightSpace, normal, lightDir);
  vec4 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * matDiffuse;

  return lighting;
}
