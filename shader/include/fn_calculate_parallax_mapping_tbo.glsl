#ifdef USE_PARALLAX
#ifdef false
vec2 calculateParallaxMappingTBO(
  uvec2 displacementTex,
  const vec2 texCoord,
  const vec3 tangentDir,
  float parallaxDepth)
{
  sampler2D displacementSampler = sampler2D(displacementTex);

  float height = texture(displacementSampler, texCoord).r;
  vec2 p = tangentDir.xy / tangentDir.z * (height * parallaxDepth);
  return texCoord - p;
}
#endif

vec2 calculateDeepParallaxMappingTBO(
  uvec2 displacementTex,
  const vec2 texCoord,
  const vec3 tangentDir,
  float parallaxDepth)
{
  sampler2D displacementSampler = sampler2D(displacementTex);

  // number of depth layers
  const float minLayers = 8.0;
  const float maxLayers = 32.0;
  const float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), tangentDir), 0.0));

  // calculate the size of each layer
  float layerDepth = 1.0 / numLayers;
  // depth of current layer
  float currentLayerDepth = 0.0;
  // the amount to shift the texture coordinates per layer (from vector P)
  vec2 P = tangentDir.xy * parallaxDepth;
  vec2 deltaTexCoord = P / numLayers;

  // get initial values
  vec2  currentTexCoord     = texCoord;
  float currentDepthMapValue = texture(displacementSampler, currentTexCoord).r;

  while(currentLayerDepth < currentDepthMapValue)
  {
    // shift texture coordinates along direction of P
    currentTexCoord -= deltaTexCoord;
    // get depthmap value at current texture coordinates
    currentDepthMapValue = texture(displacementSampler, currentTexCoord).r;
    // get depth of next layer
    currentLayerDepth += layerDepth;
  }

  return currentTexCoord;
}

vec2 calculateParallaxOcclusionMappingTBO(
  uvec2 displacementTex,
  const vec2 texCoord,
  vec3 tangentDir,
  const float parallaxDepth)
{
  sampler2D displacementSampler = sampler2D(displacementTex);

  // number of depth layers
  const float minLayers = 8.0;
  const float maxLayers = 64.0;
  const float numLayers = mix(
    maxLayers,
    minLayers,
    max(dot(vec3(0.0, 0.0, 1.0), tangentDir), 0.0));

  // calculate the size of each layer
  float layerDepth = 1.0 / numLayers;
  // depth of current layer
  float currentLayerDepth = 0.0;
  // the amount to shift the texture coordinates per layer (from vector P)
  vec2 P = -tangentDir.xy / max(tangentDir.z, 0.00001)  * parallaxDepth;
  vec2 deltaTexCoord = P / numLayers;

  // get initial values
  vec2  currentTexCoord     = texCoord;

  float currentDepthMapValue = texture(displacementSampler, currentTexCoord).r;

  for (int i = 0; i < int(numLayers); i++) {
    if (currentLayerDepth >= currentDepthMapValue) break;

    // shift texture coordinates along direction of P
    currentTexCoord -= deltaTexCoord;
    // get depthmap value at current texture coordinates
    currentDepthMapValue = texture(displacementSampler, currentTexCoord).r;
    // get depth of next layer
    currentLayerDepth += layerDepth;
  }

  // get texture coordinates before collision (reverse operations)
  vec2 prevTexCoord = currentTexCoord + deltaTexCoord;

  // get depth after and before collision for linear interpolation
  float afterDepth  = currentDepthMapValue - currentLayerDepth;
  float beforeDepth = texture(displacementSampler, prevTexCoord).r - currentLayerDepth + layerDepth;

  // interpolation of texture coordinates
  float weight = afterDepth / (afterDepth - beforeDepth);
  vec2 finalTexCoord = prevTexCoord * weight + currentTexCoord * (1.0 - weight);

  return finalTexCoord;
}

// https://www.reddit.com/r/GraphicsProgramming/comments/18qqz77/parallax_occlusion_mapping_revisited/
vec2 parallaxMapMarchTBO(
  uvec2 displacementTex,
  const vec3 tangentDir,
  const float parallaxDepth,
  const vec2 texCoord)
{
  sampler2D displacementSampler = sampler2D(displacementTex);

  vec2 pomUV = texCoord, optimalUV = texCoord;

  vec2 tanSpaceMarchDir = normalize(tangentDir.xy) / length(textureSize(displacementSampler, 0).xy);

  // Smaller samples at oblique angles
  tanSpaceMarchDir *= abs(normalize(tangentDir).z);

  float marchLen = length(tanSpaceMarchDir);
  float ratio = tangentDir.z / (length(tangentDir.xy) * parallaxDepth);
  float queryHeight = 0.0, calcHeight = 0.0;

  for (uint i = 0; i <= 40; i++)
  {
    pomUV += tanSpaceMarchDir;
    queryHeight = texture(displacementSampler, pomUV).r;
    calcHeight = float(i) * marchLen * ratio;

    if (queryHeight >= calcHeight)
      optimalUV = pomUV;

    if (calcHeight >= 1.0) break;
  }
  return optimalUV;
}

#endif
