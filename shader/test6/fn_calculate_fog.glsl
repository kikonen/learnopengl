vec4 calculateFog(vec4 color) {
  float dist = length(fs_in.viewVertexPos);
  float fogFactor = clamp(((fogEnd - dist) / (fogEnd - fogStart)), 0.0, 1.0);

  return mix(fogColor, color, pow(fogFactor, 5));
}
