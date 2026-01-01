vec4 calculateFog(
  in vec3 viewPos,
  in vec4 color)
{
  float dist = length(viewPos);
  //float fogFactor = clamp(((u_fogEnd - dist) / (u_fogEnd - u_fogStart)), 0.0, 1.0);
  float distRatio = 4.0 * dist / u_fogEnd;
  float fogFactor = clamp(exp(-distRatio * u_fogDensity * distRatio * u_fogDensity), 0.0, 1.0);

// #Ifdef USE_BLEND
// //  return mix(u_fogColor, color, pow(fogFactor, 5));
//   return vec4(mix(u_fogColor.xyz, color.xyx, fogFactor), color.a);
// #else
// //  return vec4(mix(u_fogColor, color, pow(fogFactor, 5)).xyz, 1.0);
//   return vec4(mix(u_fogColor.xyz, color.xyz, fogFactor), 1.0);
// #endif

  return vec4(mix(u_fogColor.xyz, color.xyz, fogFactor), color.a);
}
