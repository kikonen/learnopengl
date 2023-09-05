#ifdef USE_PARALLAX
vec2 calculateParallaxMapping(
  vec2 texCoord,
  vec3 viewTangentDir)
{
  sampler2D sampler = sampler2D(material.metalMapTex);
  float height = texture(sampler, texCoord).b;
  vec2 p = viewTangentDir.xy / viewTangentDir.z * (height * material.parallaxDepth);
  return texCoord - p;
}

vec2 calculateDeepParallaxMapping(
  vec2 texCoord,
  vec3 viewTangentDir)
{
  sampler2D sampler = sampler2D(material.metalMapTex);

  // number of depth layers
  const float minLayers = 8.0;
  const float maxLayers = 32.0;
  const float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewTangentDir), 0.0));

  // calculate the size of each layer
  float layerDepth = 1.0 / numLayers;
  // depth of current layer
  float currentLayerDepth = 0.0;
  // the amount to shift the texture coordinates per layer (from vector P)
  vec2 P = viewTangentDir.xy * material.parallaxDepth;
  vec2 deltaTexCoord = P / numLayers;

  // get initial values
  vec2  currentTexCoord     = texCoord;
  float currentDepthMapValue = texture(sampler, currentTexCoord).b;

  while(currentLayerDepth < currentDepthMapValue)
  {
    // shift texture coordinates along direction of P
    currentTexCoord -= deltaTexCoord;
    // get depthmap value at current texture coordinates
    currentDepthMapValue = texture(sampler, currentTexCoord).b;
    // get depth of next layer
    currentLayerDepth += layerDepth;
  }

  return currentTexCoord;
}

vec2 calculateParallaxOcclusionMapping(
  vec2 texCoord,
  vec3 viewTangentDir)
{
  sampler2D sampler = sampler2D(material.metalMapTex);

  // number of depth layers
  const float minLayers = 8.0;
  const float maxLayers = 32.0;
  const float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewTangentDir), 0.0));

  // calculate the size of each layer
  float layerDepth = 1.0 / numLayers;
  // depth of current layer
  float currentLayerDepth = 0.0;
  // the amount to shift the texture coordinates per layer (from vector P)
  vec2 P = viewTangentDir.xy * material.parallaxDepth;
  vec2 deltaTexCoord = P / numLayers;

  // get initial values
  vec2  currentTexCoord     = texCoord;
  float currentDepthMapValue = texture(sampler, currentTexCoord).b;

  while(currentLayerDepth < currentDepthMapValue)
  {
    // shift texture coordinates along direction of P
    currentTexCoord -= deltaTexCoord;
    // get depthmap value at current texture coordinates
    currentDepthMapValue = texture(sampler, currentTexCoord).b;
    // get depth of next layer
    currentLayerDepth += layerDepth;
  }

// get texture coordinates before collision (reverse operations)
  vec2 prevTexCoord = currentTexCoord + deltaTexCoord;

// get depth after and before collision for linear interpolation
  float afterDepth  = currentDepthMapValue - currentLayerDepth;
  float beforeDepth = texture(sampler, prevTexCoord).b - currentLayerDepth + layerDepth;

// interpolation of texture coordinates
  float weight = afterDepth / (afterDepth - beforeDepth);
  vec2 finalTexCoord = prevTexCoord * weight + currentTexCoord * (1.0 - weight);

  return finalTexCoord;
}

#endif
