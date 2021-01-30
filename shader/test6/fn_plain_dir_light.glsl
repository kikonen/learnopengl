float calcShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz;// / fragPosLightSpace.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;

  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  //float closestDepth = texture(shadowMap, projCoords.xy).r;

  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;

  // check whether current frag pos is in shadow
  float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

  // check whether current frag pos is in shadow
  // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
  // PCF
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
  for (int x = -1; x <= 1; ++x) {
    for(int y = -1; y <= 1; ++y) {
      float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
      shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;

  // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
  if (projCoords.z > 1.0) {
    shadow = 0.0;
  }

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
