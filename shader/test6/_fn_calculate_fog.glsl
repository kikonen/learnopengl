vec4 calculateFog(float fogRatio, vec4 color) {
  if (fogRatio == 0) return color;

  float dist = length(fs_in.viewVertexPos);
  float fogFactor = clamp(((iFogEnd - dist) / (iFogEnd - iFogStart)), 0.0, 1.0);

#ifdef USE_BLEND
  return mix(iFogColor, color, fogRatio * pow(fogFactor, 5));
#else
  return vec4(mix(iFogColor, color, fogRatio * pow(fogFactor, 5)).xyz, 1.0);
#endif
}
