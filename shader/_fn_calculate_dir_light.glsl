float lookup(
  in vec4 shadowPos,
  in float x,
  in float y,
  in float bias)
{
  // NOtE KI textureProj == automatic p.xyz / p.w
  float t = textureProj(u_shadowMap,
                        shadowPos + vec4(x * 0.001 * shadowPos.w,
                                                 y * 0.001 * shadowPos.w,
                                                 -0.01, 0.0),
                        bias);
  return t;
}

float calcShadow(
  in vec4 shadowPos,
  in vec3 normal,
  in vec3 toLight)
{
  float bias = max(0.05 * (1.0 - dot(normal, toLight)), 0.005);

  float swidth = 0.5;
  vec2 o = mod(floor(gl_FragCoord.xy), 2.0) * swidth;

  float d1 = 0.5 * swidth;
  float d2 = 1.5 * swidth;

  float shadowFactor = 0.0;
  shadowFactor += lookup(shadowPos, -d2 + o.x,  d2 - o.y, bias);
  shadowFactor += lookup(shadowPos, -d2 + o.x, -d1 - o.y, bias);
  shadowFactor += lookup(shadowPos,  d1 + o.x,  d2 - o.y, bias);
  shadowFactor += lookup(shadowPos,  d1 + o.x, -d1 - o.y, bias);
  shadowFactor = shadowFactor / 4.0;

  return shadowFactor;
}

float calcShadow2(
  in vec4 shadowPos,
  in vec3 normal,
  in vec3 toLight)
{
  float bias = max(0.05 * (1.0 - dot(normal, toLight)), 0.005);
  // NOtE KI textureProj == automatic p.xyz / p.w
  return textureProj(u_shadowMap, shadowPos, bias);
}

vec4 calculateDirLight(
  in DirLight light,
  in vec3 normal,
  in vec3 toView,
  in vec4 shadowPos,
  in Material material)
{
  const vec3 toLight = normalize(-light.worldDir);

  // ambient
  const vec4 ambient = light.ambient * material.ambient * material.diffuse;

  // diffuse
  float diff = max(dot(normal, toLight), 0.0);
  vec4 diffuse = diff * light.diffuse * material.diffuse;

  // specular
  vec4 specular = vec4(0);
  if (material.shininess > 0) {
    vec3 reflectDir = reflect(-toLight, normal);
    float spec = pow(max(dot(toView, reflectDir), 0.0), material.shininess);
    specular = spec * light.specular * material.specular;
  }

  // calculate shadow
  float shadow = calcShadow(shadowPos, normal, toLight);
  // if (shadow != 0.0) {
  //   shadow = 1;
  // }
  vec4 lighting = ambient + shadow * (diffuse + specular);

  return lighting;
}
