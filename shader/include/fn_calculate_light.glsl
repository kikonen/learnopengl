vec4 calculateLightPbr(
  const vec3 viewNormal,
  const vec3 viewPos,
  const vec3 worldPos,
  const uint shadowIndex)
{
  const mat3 invViewMat3 = mat3(u_invViewMatrix);
  const vec3 viewDir = -normalize(viewPos);

  // transform view-space normal to world-space for cubemap sampling
  const vec3 worldNormal = normalize(invViewMat3 * viewNormal);

  const vec3 N = viewNormal;
  const vec3 V = viewDir;
  const vec3 R = reflect(-V, N);

  vec3 Lo = vec3(0.0);

  const uint lightCount = u_dirLightCount + u_pointLightCount + u_spotLightCount;
  if (lightCount == 0) {
    return material.diffuse;
  }

  for (int i = 0; i < u_dirLightCount; i++) {
    //vec3 Lo = vec3(0.0);
    Lo += calculateDirLightPbr(
      u_dirLights[i],
      viewNormal,
      viewDir,
      worldPos,
      shadowIndex);
  }

  for (int i = 0; i < u_pointLightCount; i++) {
    Lo += calculatePointLightPbr(
      u_pointLights[i],
      viewNormal,
      viewDir,
      viewPos,
      shadowIndex);
  }

  for (int i = 0; i < u_spotLightCount; i++) {
    Lo += calculateSpotLightPbr(
      u_spotLights[i],
      viewNormal,
      viewDir,
      viewPos,
      shadowIndex);
  }

  // ambient lighting (we now use IBL as the ambient term)
  vec3 ambient = vec3(0.0);
  if (true) {
    vec3 albedo = material.diffuse.rgb;
    float metallic = MATERIAL_MRA_METALNESS;
    float roughness = MATERIAL_MRA_ROUGHNESS;
    float ao = MATERIAL_MRA_OCCLUSION;

#ifndef PASS_FORWARD
    if (Debug.u_ssaoBaseColorEnabled) {
      albedo = Debug.u_ssaoBaseColor.rgb;
      metallic = 0;
      roughness = 1;
      ao = 1;
    }
    if (u_cameraSsaoEnabled && Debug.u_ssaoEnabled) {
      ao = min(material.ssao, ao);
    }
#endif

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // ambient lighting (we now use IBL as the ambient term)
    const vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    const vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    const vec3 irradiance = textureLod(u_irradianceMap, worldNormal, 0).rgb;
    const vec3 diffuse = irradiance * albedo;

    // sample both the pre-filter map and the BRDF lut and combine them together
    // as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    // transform to world space for prefilter sampling
    const vec3 worldR = normalize(invViewMat3 * R);
    const vec3 prefilteredColor = textureLod(u_prefilterMap, worldR,  roughness * MAX_REFLECTION_LOD).rgb;

    const vec2 brdf  = textureLod(u_brdfLut, vec2(max(dot(N, V), 0.0), roughness), 0).rg;

    const vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    ambient = (kD * diffuse + specular) * ao;
  }

  // vec3 color = ambient + Lo;
  vec3 color = ambient + Lo + material.emission.rgb;
  //color = Lo + material.emission.rgb;
  //color = Lo;
  //Color = ambient;
  //color = material.diffuse.rgb;

  // float v = material.mras.g;
  // color = vec3(v, v, v);
  // color = material.mras.rgb;
  // color = viewNormal;

  // NOTE KI keep blending from material
  return vec4(color, material.diffuse.a);
}
