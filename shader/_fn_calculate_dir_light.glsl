
float lookup(
  in uint shadowIndex,
  in vec4 shadowPos,
  in float x,
  in float y,
  in float bias)
{
  // NOtE KI textureProj == automatic p.xyz / p.w
  float t = textureProj(u_shadowMap[shadowIndex],
                        shadowPos + vec4(x * 0.001 * shadowPos.w,
                                         y * 0.001 * shadowPos.w,
                                         -0.01,
                                         0.0),
                        bias);
  return t;
}

float calcShadow(
  in uint shadowIndex,
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
  shadowFactor += lookup(shadowIndex, shadowPos, -d2 + o.x,  d2 - o.y, bias);
  shadowFactor += lookup(shadowIndex, shadowPos, -d2 + o.x, -d1 - o.y, bias);
  shadowFactor += lookup(shadowIndex, shadowPos,  d1 + o.x,  d2 - o.y, bias);
  shadowFactor += lookup(shadowIndex, shadowPos,  d1 + o.x, -d1 - o.y, bias);
  shadowFactor = shadowFactor / 4.0;

  return shadowFactor;
}

float calcShadow2(
  in uint shadowIndex,
  in vec4 shadowPos,
  in vec3 normal,
  in vec3 toLight)
{
  float bias = max(0.05 * (1.0 - dot(normal, toLight)), 0.005);
  // NOtE KI textureProj == automatic p.xyz / p.w
  return textureProj(u_shadowMap[shadowIndex], shadowPos, bias);
}

/*
float calcShadow3(
  in uint shadowIndex,
  in vec4 shadowPos,
  in vec3 normal,
  in vec3 toLight)
{
  vec3 projCoords = shadowPos.xyz / shadowPos.w;
  projCoords = projCoords * 0.5 + 0.5;
  float currentDepth = projCoords.z;

  // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
  if (currentDepth > 1.0) {
    return 1.0;
  }

  // calculate bias (based on depth map resolution and slope)
  float bias = max(0.05 * (1.0 - dot(normal, toLight)), 0.005);
  const float biasModifier = 0.5f;

  bias *= 1 / (u_shadowPlanes[shadowIndex + 1].z * biasModifier);

  // PCF
  float shadow = 0.0;

  vec2 texelSize = 1.0 / vec2(textureSize(u_shadowMap[shadowIndex], 0));

  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      // float pcfDepth = texture(u_shadowMap[shadowIndex], vec3(projCoords.xy + vec2(x, y) * texelSize, 1)).r;

      float pcfDepth =
        texture(
                u_shadowMap[shadowIndex],
                projCoords.xy + vec2(x, y) * texelSize).r;

      shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;

  return 1.0 - shadow;
}
*/

float calcShadow4(
  in uint shadowIndex,
  in vec4 shadowPos,
  in vec3 normal,
  in vec3 toLight)
{
  // calculate bias (based on depth map resolution and slope)
  float bias = max(0.05 * (1.0 - dot(normal, toLight)), 0.005);
  const float biasModifier = 0.5f;

  bias *= 1 / (u_shadowPlanes[shadowIndex + 1].z * biasModifier);

  // PCF
  float shadow = 0.0;

  const float w = shadowPos.w;
  const vec2 texelSize = (1.0 / vec2(textureSize(u_shadowMap[shadowIndex], 0))) / w;

  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      float pcf =
        textureProj(
                u_shadowMap[shadowIndex],
                shadowPos + vec4(x * texelSize.x, y * texelSize.y, 0.0, 0.0),
                bias).r;

      shadow += pcf;
    }
  }
  shadow /= 9.0;

  return shadow;
}

vec4 calculateDirLight(
  in DirLight light,
  in vec3 normal,
  in vec3 toView,
  in uint shadowIndex,
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
  float shadow = calcShadow(shadowIndex, shadowPos, normal, toLight);
  // if (shadow != 0.0) {
  //   shadow = 1;
  // }
  vec4 lighting = ambient + shadow * (diffuse + specular);

  return lighting;
}
