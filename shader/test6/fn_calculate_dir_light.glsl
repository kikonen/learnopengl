float lookup(vec4 pos, float x, float y, float bias)
{
  float t = textureProj(shadowMap,
                        pos + vec4(x * 0.001 * pos.w,
                                   y * 0.001 * pos.w,
                                   -0.01, 0.0),
                        bias);
  return t;
}

float calcShadow(vec4 pos, vec3 normal, vec3 lightDir)
{
  float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

  float swidth = 2.5;
  vec2 o = mod(floor(gl_FragCoord.xy), 2.0) * swidth;

  float shadowFactor = 0.0;
  shadowFactor += lookup(pos, -1.5*swidth + o.x,  1.5*swidth - o.y, bias);
  shadowFactor += lookup(pos, -1.5*swidth + o.x, -0.5*swidth - o.y, bias);
  shadowFactor += lookup(pos, 0.5*swidth + o.x,  1.5*swidth - o.y, bias);
  shadowFactor += lookup(pos, 0.5*swidth + o.x, -0.5*swidth - o.y, bias);
  shadowFactor = shadowFactor / 4.0;

  return shadowFactor;
}

vec4 calculateDirLight(
  DirLight light,
  vec3 normal,
  vec3 viewDir,
  vec4 fragPosLightSpace,
  Material material)
{
  vec3 lightDir = normalize(-light.dir);

  // ambient
  vec4 ambient = light.ambient * material.ambient;

  // diffuse
  vec4 diffuse = max(dot(normal, lightDir), 0.0) * light.diffuse * material.diffuse;

  // specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  vec4 specular = spec * light.specular * material.specular;

  // calculate shadow
  float shadow = calcShadow(fragPosLightSpace, normal, lightDir);
  //shadow = 0;
  vec4 lighting = ambient + shadow * (diffuse + specular);

  return lighting;
  //return ambient + diffuse + specular;
}
