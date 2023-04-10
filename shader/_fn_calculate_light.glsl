vec4 calculateLight(
  in vec3 normal,
  in vec3 toView,
  in vec3 worldPos,
  in uint shadowIndex,
  in vec4 shadowPos,
  in Material material)
{
  const vec4 emission = material.emission;

  // https://community.khronos.org/t/default-value-of-uninitialized-local-variable-and-uniforms/74701/2
  // TODO KI if dir light not defined need to have some ambient light
  vec4 dirShaded = material.diffuse;
  vec4 pointShaded = vec4(0);
  vec4 spotShaded = vec4(0);

  uint lightCount = u_dirLightCount + u_pointLightCount + u_spotLightCount;

  for (int i = 0; i < u_dirLightCount; i++) {
    dirShaded = calculateDirLight(u_dirLights[i], normal, toView, shadowIndex, shadowPos, material);
  }

  for (int i = 0; i < u_pointLightCount; i++) {
    float dist = length(u_pointLights[i].worldPos - worldPos);
    if (dist < u_pointLights[i].radius) {
      pointShaded += calculatePointLight(u_pointLights[i], normal, toView, worldPos, material);
    }
  }

  for (int i = 0; i < u_spotLightCount; i++) {
    float dist = length(u_spotLights[i].worldPos - worldPos);
    if (dist < u_spotLights[i].radius) {
      spotShaded += calculateSpotLight(u_spotLights[i], normal, toView, worldPos, material);
    }
  }

  vec4 shaded;
  if (lightCount > 0) {
    shaded = dirShaded + pointShaded + spotShaded + emission;
  } else {
    shaded = material.diffuse;// + material.emission;
  }
  // NOTE KI keep blending from material
  shaded.a = material.diffuse.a;

  return shaded;
}
