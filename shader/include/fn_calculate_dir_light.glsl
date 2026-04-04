// Poisson disk samples for PCF
const vec2 POISSON_DISK[16] = vec2[](
  vec2(-0.94201624, -0.39906216),
  vec2( 0.94558609, -0.76890725),
  vec2(-0.09418410, -0.92938870),
  vec2( 0.34495938,  0.29387760),
  vec2(-0.91588581,  0.45771432),
  vec2(-0.81544232, -0.87912464),
  vec2(-0.38277543,  0.27676845),
  vec2( 0.97484398,  0.75648379),
  vec2( 0.44323325, -0.97511554),
  vec2( 0.53742981, -0.47373420),
  vec2(-0.26496911, -0.41893023),
  vec2( 0.79197514,  0.19090188),
  vec2(-0.24188840,  0.99706507),
  vec2(-0.81409955,  0.91437590),
  vec2( 0.19984126,  0.78641367),
  vec2( 0.14383161, -0.14100790)
  );

#define SHADOW_PCF_SAMPLES 4
#define SHADOW_PCF_SPREAD 1.25

float calcShadowPCF(
  const sampler2DShadow shadowMap,
  const vec4 shadowPos,
  const float spread)
{
  const vec3 sp = shadowPos.xyz / shadowPos.w;
  if (any(lessThan(sp, vec3(0.0))) || any(greaterThan(sp, vec3(1.0)))) {
    return 1.0;
  }

  // get shadow map texel size
  const vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

  float shadow = 0.0;
  // 8 or 16 depending on quality needs
  const int samples = SHADOW_PCF_SAMPLES;
  for (int i = 0; i < samples; i++) {
    vec4 offset = vec4(POISSON_DISK[i] * texelSize * spread, 0.0, 0.0);
    shadow += textureProj(shadowMap, shadowPos + offset);
  }
  return shadow / float(samples);
}

bool inShadowRange(float v)
{
  return v >= 0.0 && v <= 1.0;
}

// NOTE KI
// https://computergraphics.stackexchange.com/questions/4354/exponential-shadow-maps-sampling-with-pcf-for-sampler2dshadow-instead-of-sampler
float calcShadow(
  const sampler2DShadow shadowMap,
  const vec4 shadowPos)
{
  // "shadow band" issue
  // https://www.youtube.com/watch?v=dwMcE8_Mt8U&t=320s
  // if (!(inShadowRange(shadowPos.x) ||
  // 	inShadowRange(shadowPos.y) ||
  // 	inShadowRange(shadowPos.z)))
  // {
  //   return 1.0;
  // }

  // correct -- outside frustum if ANY component out of range
  const vec3 sp = shadowPos.xyz;
  if (any(lessThan(sp, vec3(0.0))) || any(greaterThan(sp, vec3(1.0)))) {
    return 1.0;
  }

  // NOTE KI using glPolygonOffset
  // With GL_LINEAR & sampler2dshadow & textureProj
  // => free HW PCF
  return textureProj(shadowMap, shadowPos);
}

vec3 calculateDirLightPbr(
  const DirLight light,
  const vec3 viewNormal,
  const vec3 viewDir,
  const vec3 worldPos,
  const uint shadowIndex)
{
  const float DIR_LIGHT_REF_DIST = 100.0;
  const float DIR_LIGHT_SCALE = 1.0 / (DIR_LIGHT_REF_DIST * DIR_LIGHT_REF_DIST);

  const vec3 toLight = -normalize(mat3(u_viewMatrix) * light.worldDir.xyz);
  // const float lightDistance = 100.0;

  const vec3 N = viewNormal;
  const vec3 V = viewDir;

  const vec3 albedo = material.diffuse.rgb;
  const float metallic = MATERIAL_MRA_METALNESS;
  const float roughness = MATERIAL_MRA_ROUGHNESS;

  // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
  // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  // reflectance equation
  vec3 Lo = vec3(0.0);
  {
    // calculate per-light radiance
    const vec3 L = toLight;
    const vec3 H = normalize(V + L);
    // const float lightDistance = lightdist;
    // const float attenuation = 1.0 / (lightDistance * lightDistance);
    const float attenuation = DIR_LIGHT_SCALE;
    const vec3 radiance = light.diffuse.rgb * light.diffuse.a * attenuation;

    // Cook-Torrance BRDF
    const float NDF = distributionGGX(N, H, roughness);
    const float G   = geometrySmith(N, V, L, roughness);
    const vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    const vec3 numerator    = NDF * G * F;
    // + 0.0001 to prevent divide by zero
    const float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    const vec3 specular = numerator / denominator;

    // kS is equal to Fresnel
    const vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;

    // scale light by NdotL
    const float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  }

  // calculate shadow
  const vec4 shadowPos = u_shadowMatrix[shadowIndex] * vec4(worldPos, 1.0);
  // const float shadow = calcShadow(u_shadowMap[shadowIndex], shadowPos);

  // spread controls softness -- tune to taste
  const float shadow = calcShadowPCF(
    u_shadowMap[shadowIndex],
    shadowPos,
    SHADOW_PCF_SPREAD);

  //return viewNormal;
  //Lo = vec3(1, 0, 0);
  return shadow * Lo;
}
