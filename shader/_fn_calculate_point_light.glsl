vec3 calculatePointLight(
  in PointLight light,
  in vec3 normal,
  in vec3 viewDir,
  in vec3 worldPos)
{
  const vec3 toLight = light.worldPos.xyz - worldPos;
  const float dist = length(toLight);

  if (dist > light.radius) return vec3(0.0);

  const vec3 lightDir = normalize(toLight);

  const float powerup = 1.7;

  // diffuse
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = powerup * light.diffuse.rgb * (diff * material.diffuse.rgb);

  // specular
  vec3 specular = vec3(0);
  const float shininess = material.specular.a;
  if (shininess > 0) {
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    specular = powerup * light.diffuse.rgb * (spec * material.specular.rgb);
  }

  float attenuation = 1.0 / (light.constant + light.linear * dist +
                             light.quadratic * (dist * dist));
  diffuse  *= attenuation;
  specular *= attenuation;

  return diffuse + specular;
}

vec3 calculatePointLightPbr(
  in PointLight light,
  in vec3 normal,
  in vec3 viewDir,
  in vec3 worldPos,
  in uint shadowIndex)
{
  const vec3 toLight = light.worldPos.xyz - worldPos;
  const float dist = length(toLight);

  if (dist > light.radius) return vec3(0.0);

  const vec3 lightDir = normalize(toLight);
  const vec3 lightPos = light.worldPos.xyz;

  const vec3 N = normal;
  const vec3 V = viewDir;

  const vec3 albedo = material.diffuse.rgb;
  const float metallic = material.metal.r;
  const float roughness = material.metal.g;

  // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
  // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, material.diffuse.rgb, metallic);

  // reflectance equation
  vec3 Lo = vec3(0.0);
  {
    // calculate per-light radiance
    vec3 L = lightDir; //normalize(light.worldPos - worldPos);
    vec3 H = normalize(V + L);
    float distance = length(lightPos - worldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = light.diffuse.rgb * light.diffuse.a * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
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
    Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
  }

  return Lo;
}
