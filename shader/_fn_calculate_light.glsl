vec4 calculateLight(
  in vec3 normal,
  in vec3 viewDir,
  in vec3 worldPos,
  in uint shadowIndex,
  in Material material)
{
  // https://community.khronos.org/t/default-value-of-uninitialized-local-variable-and-uniforms/74701/2
  vec3 color = vec3(0);

  const uint lightCount = u_dirLightCount + u_pointLightCount + u_spotLightCount;
  if (lightCount == 0) {
    return material.diffuse;
  }

  for (int i = 0; i < u_dirLightCount; i++) {
    color += calculateDirLight(
      u_dirLights[i],
      normal,
      viewDir,
      worldPos,
      shadowIndex,
      material);
  }

  for (int i = 0; i < u_pointLightCount; i++) {
    color += calculatePointLight(
      u_pointLights[i],
      normal,
      viewDir,
      worldPos,
      material);
  }

  for (int i = 0; i < u_spotLightCount; i++) {
    color += calculateSpotLight(
      u_spotLights[i],
      normal,
      viewDir,
      worldPos,
      material);
  }

  vec3 shaded = (material.ambient * material.diffuse.xyz) +
    color +
    material.emission.xyz;

  // NOTE KI keep blending from material
  return vec4(shaded, material.diffuse.a);
}
