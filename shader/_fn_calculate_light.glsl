vec4 calculateLight(
  in vec3 normal,
  in vec3 viewDir,
  in vec3 worldPos,
  in uint shadowIndex)
{
  // https://community.khronos.org/t/default-value-of-uninitialized-local-variable-and-uniforms/74701/2
  vec3 color = vec3(0);

  const uint lightCount = u_dirLightCount + u_pointLightCount + u_spotLightCount;
  if (lightCount == 0) {
    return material.diffuse;
  }

  for (int i = 0; i < u_dirLightCount; i++) {
    color += calculateDirLight(
      u_dirLights[i],
      normal,
      viewDir,
      worldPos,
      shadowIndex);
  }

  for (int i = 0; i < u_pointLightCount; i++) {
    color += calculatePointLight(
      u_pointLights[i],
      normal,
      viewDir,
      worldPos);
  }

  for (int i = 0; i < u_spotLightCount; i++) {
    color += calculateSpotLight(
      u_spotLights[i],
      normal,
      viewDir,
      worldPos);
  }

  vec3 shaded = (material.ambient * material.diffuse.rgb) +
    color +
    material.emission.rgb;

  // NOTE KI keep blending from material
  return vec4(shaded, material.diffuse.a);
}

vec4 calculateLightPbr(
  in vec3 normal,
  in vec3 viewDir,
  in vec3 worldPos,
  in uint shadowIndex)
{
  const vec3 N = normal;
  const vec3 V = viewDir;

  vec3 Lo = vec3(0.0);

  const uint lightCount = u_dirLightCount + u_pointLightCount + u_spotLightCount;
  if (lightCount == 0) {
    return material.diffuse;
  }

  for (int i = 0; i < u_dirLightCount; i++) {
    //vec3 Lo = vec3(0.0);
    Lo += calculateDirLightPbr(
      u_dirLights[i],
      normal,
      viewDir,
      worldPos,
      shadowIndex);
  }

  for (int i = 0; i < u_pointLightCount; i++) {
    Lo += calculatePointLightPbr(
      u_pointLights[i],
      normal,
      viewDir,
      worldPos,
      shadowIndex);
  }

  for (int i = 0; i < u_spotLightCount; i++) {
    Lo += calculateSpotLightPbr(
      u_spotLights[i],
      normal,
      viewDir,
      worldPos,
      shadowIndex);
  }

  // ambient lighting (we now use IBL as the ambient term)
  vec3 ambient = vec3(0.0);
  if (true) {
    const vec3 albedo = material.diffuse.rgb;
    const float metallic = material.metal.r;
    const float roughness = material.metal.g;
    const float ao = material.metal.a;

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 kS = fresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 irradiance = texture(u_irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    ambient = (kD * diffuse) * ao;
    // vec3 ambient = vec3(0.002);
  }

  vec3 color = ambient + Lo + material.emission.rgb;
  //color = Lo + material.emission.rgb;
  //color = ambient;
  //Color = ambient;
  //color = material.diffuse.rgb;

  // NOTE KI keep blending from material
  return vec4(color, material.diffuse.a);
}
