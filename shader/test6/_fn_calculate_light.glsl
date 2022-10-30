vec4 calculateLight(
  vec3 normal,
  vec3 toView,
  Material material)
{
  bool hasLight = false;
  vec4 emission = material.emission;

  // https://community.khronos.org/t/default-value-of-uninitialized-local-variable-and-uniforms/74701/2
  // TODO KI if dir light not defined need to have some ambient light
  vec4 dirShaded = material.diffuse;
  vec4 pointShaded = vec4(0);
  vec4 spotShaded = vec4(0);

  if (u_light.use) {
    dirShaded = calculateDirLight(u_light, normal, toView, fs_in.fragPosLightSpace, material);
    hasLight = true;
  }

  for (int i = 0; i < LIGHT_COUNT; i++) {
    if (u_pointLights[i].use) {
      float dist = length(u_pointLights[i].pos - fs_in.fragPos);
      if (dist < u_pointLights[i].radius) {
        pointShaded += calculatePointLight(u_pointLights[i], normal, toView, fs_in.fragPos, material);
        hasLight = true;
      }
    }
  }

  for (int i = 0; i < LIGHT_COUNT; i++) {
    if (u_spotLights[i].use) {
      float dist = length(u_spotLights[i].pos - fs_in.fragPos);
      if (dist < u_spotLights[i].radius) {
        spotShaded += calculateSpotLight(u_spotLights[i], normal, toView, fs_in.fragPos, material);
        hasLight = true;
      }
    }
  }

  vec4 shaded;
  if (hasLight) {
    shaded = dirShaded + pointShaded + spotShaded + emission;
  } else {
    shaded = material.diffuse;// + material.emission;
  }
  // NOTE KI keep blending from material
  shaded.a = material.diffuse.a;

  return shaded;
}
