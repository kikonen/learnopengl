#ifdef USE_PARALLAX
vec2 calculateParallaxMapping(
  const uint materialIndex,
  const vec2 texCoord,
  const vec3 viewTangentDir,
  float parallaxDepth)
{
  sampler2D sampler = sampler2D(u_materials[materialIndex].displacementMapTex);
  float height = texture(sampler, texCoord).r;
  vec2 p = viewTangentDir.xy / viewTangentDir.z * (height * parallaxDepth);
  return texCoord - p;
}

vec2 calculateDeepParallaxMapping(
  const uint materialIndex,
  const vec2 texCoord,
  const vec3 viewTangentDir,
  float parallaxDepth)
{
  sampler2D sampler = sampler2D(u_materials[materialIndex].displacementMapTex);

  // number of depth layers
  const float minLayers = 8.0;
  const float maxLayers = 32.0;
  const float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewTangentDir), 0.0));

  // calculate the size of each layer
  float layerDepth = 1.0 / numLayers;
  // depth of current layer
  float currentLayerDepth = 0.0;
  // the amount to shift the texture coordinates per layer (from vector P)
  vec2 P = viewTangentDir.xy * parallaxDepth;
  vec2 deltaTexCoord = P / numLayers;

  // get initial values
  vec2  currentTexCoord     = texCoord;
  float currentDepthMapValue = texture(sampler, currentTexCoord).r;

  while(currentLayerDepth < currentDepthMapValue)
  {
    // shift texture coordinates along direction of P
    currentTexCoord -= deltaTexCoord;
    // get depthmap value at current texture coordinates
    currentDepthMapValue = texture(sampler, currentTexCoord).r;
    // get depth of next layer
    currentLayerDepth += layerDepth;
  }

  return currentTexCoord;
}

vec2 calculateParallaxOcclusionMapping(
  const uint materialIndex,
  const vec2 texCoord,
  const vec3 viewTangentDir,
  float parallaxDepth)
{
  sampler2D sampler = sampler2D(u_materials[materialIndex].displacementMapTex);

  // number of depth layers
  const float minLayers = 8.0;
  const float maxLayers = 32.0;
  const float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewTangentDir), 0.0));

  // calculate the size of each layer
  float layerDepth = 1.0 / numLayers;
  // depth of current layer
  float currentLayerDepth = 0.0;
  // the amount to shift the texture coordinates per layer (from vector P)
  vec2 P = -viewTangentDir.xy / viewTangentDir.z * parallaxDepth;
  vec2 deltaTexCoord = P / numLayers;

  // get initial values
  vec2  currentTexCoord     = texCoord;

  float currentDepthMapValue = texture(sampler, currentTexCoord).r;

  while(currentLayerDepth < currentDepthMapValue)
  {
    // shift texture coordinates along direction of P
    currentTexCoord -= deltaTexCoord;
    // get depthmap value at current texture coordinates
    currentDepthMapValue = texture(sampler, currentTexCoord).r;
    // get depth of next layer
    currentLayerDepth += layerDepth;
  }

// get texture coordinates before collision (reverse operations)
  vec2 prevTexCoord = currentTexCoord + deltaTexCoord;

// get depth after and before collision for linear interpolation
  float afterDepth  = currentDepthMapValue - currentLayerDepth;
  float beforeDepth = texture(sampler, prevTexCoord).r - currentLayerDepth + layerDepth;

// interpolation of texture coordinates
  float weight = afterDepth / (afterDepth - beforeDepth);
  vec2 finalTexCoord = prevTexCoord * weight + currentTexCoord * (1.0 - weight);

  return finalTexCoord;
}

// https://www.reddit.com/r/GraphicsProgramming/comments/18qqz77/parallax_occlusion_mapping_revisited/
vec2 parallaxMapMarch (
  uint materialIndex,
  vec3 viewTangentDir,
  float parallaxDepth,
  vec2 texCoord)
{
  sampler2D sampler = sampler2D(u_materials[materialIndex].displacementMapTex);

  vec2 pomUV = texCoord, optimalUV = texCoord;

  vec2 tanSpaceMarchDir = normalize(viewTangentDir.xy) / length(textureSize(sampler, 0).xy);

  // Smaller samples at oblique angles
  tanSpaceMarchDir *= abs(normalize(viewTangentDir).z);

  float marchLen = length(tanSpaceMarchDir);
  float ratio = viewTangentDir.z / (length(viewTangentDir.xy) * parallaxDepth);
  float queryHeight = 0.0, calcHeight = 0.0;

  for (uint i = 0; i <= 40; i++)
  {
    pomUV += tanSpaceMarchDir;
    queryHeight = texture(sampler, pomUV).r;
    calcHeight = float(i) * marchLen * ratio;

    if (queryHeight >= calcHeight)
      optimalUV = pomUV;

    if (calcHeight >= 1.0) break;
  }
  return optimalUV;
}

#endif
