vec4 calculateFog(float fogRatio, vec4 color) {
  if (fogRatio == 0) return color;

  float dist = length(fs_in.viewVertexPos);
  float fogFactor = clamp(((fogEnd - dist) / (fogEnd - fogStart)), 0.0, 1.0);

  return mix(fogColor, color, fogRatio * pow(fogFactor, 5));
}
