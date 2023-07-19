// NOTE KI
// https://computergraphics.stackexchange.com/questions/4354/exponential-shadow-maps-sampling-with-pcf-for-sampler2dshadow-instead-of-sampler
float calcShadow2_5(
  in sampler2DShadow shadowMap,
  in vec4 shadowPos)
{
  if (shadowPos.z > 1.0) return 0.0;

  // NOTE KI using glPolygonOffset
  // With GL_LINEAR & sampler2dshadow & textureProj
  // => free HW PCF
  return textureProj(shadowMap, shadowPos);
}

vec3 calculateDirLight(
  in DirLight light,
  in vec3 normal,
  in vec3 toView,
  in vec3 worldPos,
  in uint shadowIndex,
  in Material material)
{
  const vec3 toLight = normalize(-light.worldDir);
  const vec4 shadowPos = u_shadowMatrix[shadowIndex] * vec4(worldPos, 1.0);

  // diffuse
  float diff = max(dot(normal, toLight), 0.0);
  vec3 diffuse = diff * light.diffuse * material.diffuse.xyz;

  // specular
  vec3 specular = vec3(0);
  const float shininess = material.specular.a;
  if (shininess > 0) {
    vec3 reflectDir = reflect(-toLight, normal);
    float spec = pow(max(dot(toView, reflectDir), 0.0), shininess);
    specular = spec * light.specular * material.specular.xyz;
  }

  // calculate shadow
  //float shadow = calcShadow2_3(worldPos, shadowIndex, shadowPos, normal, toLight);
  float shadow = calcShadow2_5(u_shadowMap[shadowIndex], shadowPos);
  shadow = clamp(shadow, 0.2, 1.0);

  return shadow * (diffuse + specular);
}
