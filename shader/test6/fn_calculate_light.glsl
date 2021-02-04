vec4 calculateLight(
  vec3 normal,
  vec3 viewDir,
  Material material)
{
  bool hasLight = false;
  vec4 emission = material.emission;

  // https://community.khronos.org/t/default-value-of-uninitialized-local-variable-and-uniforms/74701/2
  vec4 dirShaded = vec4(0);
  vec4 pointShaded = vec4(0);
  vec4 spotShaded = vec4(0);

  if (light.use) {
    dirShaded = calculateDirLight(light, normal, viewDir, fs_in.fragPosLightSpace, material);
    hasLight = true;
  }

  for (int i = 0; i < LIGHT_COUNT; i++) {
    if (pointLights[i].use) {
      pointShaded += calculatePointLight(pointLights[i], normal, viewDir, fs_in.fragPos, material);
      hasLight = true;
    }
  }

  for (int i = 0; i < LIGHT_COUNT; i++) {
    if (spotLights[i].use) {
      spotShaded += calculateSpotLight(spotLights[i], normal, viewDir, fs_in.fragPos, material);
      hasLight = true;
    }
  }

  vec4 shaded;
  if (hasLight) {
    shaded = dirShaded + pointShaded + spotShaded + emission;
  } else {
    shaded = material.diffuse;// + material.emission;
  }
  return shaded;
}
