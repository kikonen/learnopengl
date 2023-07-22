#ifdef USE_HEIGHT_TEX
vec2 calculateParallaxMapping(
  in Material material,
  vec2 texCoord,
  vec3 toView)
{
  sampler2D sampler = sampler2D(u_texture_handles[material.heightMapTex]);
  float height = texture(sampler, texCoord).r;
  vec2 p = toView.xy / toView.z * (height * material.depth);
  return texCoord - p;
}

vec2 calculateDeepParallaxMapping(
  in Material material,
  vec2 texCoord,
  vec3 toView)
{
  sampler2D sampler = sampler2D(u_texture_handles[material.heightMapTex]);

  // number of depth layers
  const float minLayers = 8.0;
  const float maxLayers = 32.0;
  const float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), toView), 0.0));

  // calculate the size of each layer
  float layerDepth = 1.0 / numLayers;
  // depth of current layer
  float currentLayerDepth = 0.0;
  // the amount to shift the texture coordinates per layer (from vector P)
  vec2 P = toView.xy * material.depth;
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
#endif
