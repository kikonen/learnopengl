// Apply animated water-caustic projection onto color. Triplanar projection in
// world space — three axis-aligned planar projections blended by surface
// orientation, so vertical walls don't get the streaks that pure XZ projection
// produces.
//
// The geometric normal is reconstructed from worldPos derivatives so callers
// only need worldPos; this also keeps the projection stable on shaders whose
// stored normal is wave-perturbed or unavailable (OIT).
//
// Requires:
//   uniform_data.glsl   (u_waterCausticEnabled, u_waterCausticMaterialIndex,
//                        u_waterCausticIntensity, u_waterCausticWorldLevel,
//                        u_waterCausticScale, u_time)
//   uniform_camera.glsl (u_cameraWaterCausticsEnabled)
//   ssbo_materials.glsl (u_materials)
vec3 _sampleWaterCausticTriplanar(vec3 worldPos) {
  vec3 worldNormal = normalize(cross(dFdx(worldPos), dFdy(worldPos)));

  vec2 offset = vec2(sin(u_time * 0.2), cos(u_time * 0.1)) * 0.3;
  vec2 uvX = worldPos.zy * u_waterCausticScale + offset;
  vec2 uvY = worldPos.xz * u_waterCausticScale + offset;
  vec2 uvZ = worldPos.xy * u_waterCausticScale + offset;

  // Sharpen the blend so vertical walls cleanly favor X/Z projections instead
  // of muddy averages with the down-projected XZ stripe.
  vec3 absN = pow(abs(worldNormal), vec3(4.0));
  vec3 blend = absN / (absN.x + absN.y + absN.z + 1e-5);

  uint matIdx = u_waterCausticMaterialIndex;
  sampler2D sampler = sampler2D(u_materials[matIdx].diffuseTex);
  vec3 cX = texture(sampler, uvX).rgb;
  vec3 cY = texture(sampler, uvY).rgb;
  vec3 cZ = texture(sampler, uvZ).rgb;

  return cX * blend.x + cY * blend.y + cZ * blend.z;
}

void applyWaterCausticAlways(inout vec3 color, vec3 worldPos) {
  if (!u_waterCausticEnabled) return;
  if (!u_cameraWaterCausticsEnabled) return;

  vec3 causticColor = _sampleWaterCausticTriplanar(worldPos);
  color = mix(color, causticColor, u_waterCausticIntensity);
}

// For surfaces strictly below the water level (terrain floor, pool walls,
// generic meshes). The level check excludes fragments above water.
void applyWaterCaustic(inout vec3 color, vec3 worldPos) {
  // NOTE KI strict less-than would flicker on the water plane itself — but
  // the water plane uses applyWaterCausticAlways instead, so we can be safe.
  if (worldPos.y >= u_waterCausticWorldLevel) return;
  applyWaterCausticAlways(color, worldPos);
}
