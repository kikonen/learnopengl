// Apply animated water-caustic projection onto color. UV is derived from world
// XZ so all surfaces tile consistently regardless of mesh UVs.
//
// Requires:
//   uniform_data.glsl   (u_waterCausticEnabled, u_waterCausticMaterialIndex,
//                        u_waterCausticIntensity, u_waterCausticWorldLevel,
//                        u_waterCausticScale, u_time)
//   uniform_camera.glsl (u_cameraWaterCausticsEnabled)
//   ssbo_materials.glsl (u_materials)
void applyWaterCausticAlways(inout vec3 color, vec3 worldPos) {
  if (!u_waterCausticEnabled) return;
  if (!u_cameraWaterCausticsEnabled) return;

  vec2 causticUV =
      worldPos.xz * u_waterCausticScale
      + vec2(sin(u_time * 0.2), cos(u_time * 0.1)) * 0.3;

  vec3 causticColor =
      texture(sampler2D(u_materials[u_waterCausticMaterialIndex].diffuseTex),
              causticUV).rgb;

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
