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
  in vec3 viewDir,
  in vec3 worldPos,
  in uint shadowIndex)
{
  const vec3 lightDir = normalize(-light.worldDir.xyz);
  const vec4 shadowPos = u_shadowMatrix[shadowIndex] * vec4(worldPos, 1.0);

  // diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diff * light.diffuse.rgb * material.diffuse.rgb;

  // specular
  vec3 specular = vec3(0);
  const float shininess = material.specular.a;
  if (shininess > 0) {
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    specular = spec * light.diffuse.rgb * material.specular.rgb;
  }

  // calculate shadow
  //float shadow = calcShadow2_3(worldPos, shadowIndex, shadowPos, normal, lightDir);
  float shadow = calcShadow2_5(u_shadowMap[shadowIndex], shadowPos);

  return shadow * (diffuse + specular);
}

vec3 calculateDirLightPbr(
  in DirLight light,
  in vec3 normal,
  in vec3 viewDir,
  in vec3 worldPos,
  in uint shadowIndex)
{
  const vec3 toLight = -light.worldDir.xyz;
  const float dist = 100.0;

  // calculate shadow
  const vec4 shadowPos = u_shadowMatrix[shadowIndex] * vec4(worldPos, 1.0);
  const float shadow = calcShadow2_5(u_shadowMap[shadowIndex], shadowPos);

  const vec3 N = normal;
  const vec3 V = viewDir;

  const vec3 albedo = material.diffuse.rgb;
  const float metallic = material.metal.r;
  const float roughness = material.metal.g;

  // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
  // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  // reflectance equation
  vec3 Lo = vec3(0.0);
  {
    // calculate per-light radiance
    vec3 L = toLight; //normalize(light.worldPos - worldPos);
    vec3 H = normalize(V + L);
    float distance = dist; // length(lightPos - worldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light.diffuse.rgb * light.diffuse.a * attenuation;

    // Cook-Torrance BRDF
    float NDF = distributionGGX(N, H, roughness);
    float G   = geometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F;
    // + 0.0001 to prevent divide by zero
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  }

  //Lo = vec3(1, 0, 0);
  return shadow * Lo;
}
