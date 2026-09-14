#ifndef USE_TEXTURE_ARRAY
#ifdef USE_PARALLAX
vec2 calculateParallaxMapping(
  const uint materialIndex,
  const vec2 texCoord,
  const vec3 tangentDir,
  float parallaxDepth)
{
  sampler2D sampler = sampler2D(readMaterial_displacementMapTex(materialIndex));
  float height = texture(sampler, texCoord).r;

  vec2 p = tangentDir.xy / tangentDir.z * (height * parallaxDepth);
  return texCoord - p;
}

vec2 calculateDeepParallaxMapping(
  const uint materialIndex,
  const vec2 texCoord,
  const vec3 tangentDir,
  float parallaxDepth)
{
  sampler2D sampler = sampler2D(readMaterial_displacementMapTex(materialIndex));

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
  vec3 tangentDir,
  const float parallaxDepth)
{
  sampler2D sampler = sampler2D(readMaterial_displacementMapTex(materialIndex));

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

  float currentDepthMapValue = texture(sampler, currentTexCoord).r;

  for (int i = 0; i < int(numLayers); i++) {
    if (currentLayerDepth >= currentDepthMapValue) break;

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
  const uint materialIndex,
  const vec3 tangentDir,
  const float parallaxDepth,
  const vec2 texCoord)
{
  sampler2D sampler = sampler2D(readMaterial_displacementMapTex(materialIndex));

  vec2 pomUV = texCoord, optimalUV = texCoord;

  const ivec2 texSize = textureSize(sampler, 0).xy;
  vec2 tanSpaceMarchDir = normalize(tangentDir.xy) / length(texSize);

  // Smaller samples at oblique angles
  tanSpaceMarchDir *= abs(normalize(tangentDir).z);

  float marchLen = length(tanSpaceMarchDir);
  float ratio = tangentDir.z / (length(tangentDir.xy) * parallaxDepth);
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
#endif

#ifdef USE_TEXTURE_ARRAY
#ifdef USE_PARALLAX
vec2 calculateParallaxMapping(
  const uint materialIndex,
  const vec2 texCoord,
  const vec3 tangentDir,
  float parallaxDepth)
{
  const int displacementLayer = int(readMaterial_displacementMapTex(materialIndex).x);

  float height = texture(
    u_texturesDisplacement,
    vec3(texCoord, float(displacementLayer))).r;

  vec2 p = tangentDir.xy / tangentDir.z * (height * parallaxDepth);
  return texCoord - p;
}

vec2 calculateDeepParallaxMapping(
  const uint materialIndex,
  const vec2 texCoord,
  const vec3 tangentDir,
  float parallaxDepth)
{
  const int displacementLayer = int(readMaterial_displacementMapTex(materialIndex).x);

  // number of depth layers
  const float minLayers = 8.0;
  const float maxLayers = 32.0;
  const float numLayers = mix(
    maxLayers,
    minLayers,
    max(dot(vec3(0.0, 0.0, 1.0), tangentDir), 0.0));

  // calculate the size of each layer
  float layerDepth = 1.0 / numLayers;
  // depth of current layer
  float currentLayerDepth = 0.0;
  // the amount to shift the texture coordinates per layer (from vector P)
  vec2 P = tangentDir.xy * parallaxDepth;
  vec2 deltaTexCoord = P / numLayers;

  // get initial values
  vec2  currentTexCoord     = texCoord;
  float currentDepthMapValue = texture(
    u_texturesDisplacement,
    vec3(currentTexCoord, float(displacementLayer))).r;

  while(currentLayerDepth < currentDepthMapValue)
  {
    // shift texture coordinates along direction of P
    currentTexCoord -= deltaTexCoord;

    // get depthmap value at current texture coordinates
    currentDepthMapValue = texture(
      u_texturesDisplacement,
      vec3(currentTexCoord, float(displacementLayer))).r;

    // get depth of next layer
    currentLayerDepth += layerDepth;
  }

  return currentTexCoord;
}

vec2 calculateParallaxOcclusionMapping(
  const uint materialIndex,
  const vec2 texCoord,
  vec3 tangentDir,
  const float parallaxDepth)
{
  const int displacementLayer = int(readMaterial_displacementMapTex(materialIndex).x);

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

  float currentDepthMapValue = texture(
    u_texturesDisplacement,
    vec3(currentTexCoord, float(displacementLayer))).r;

  for (int i = 0; i < int(numLayers); i++) {
    if (currentLayerDepth >= currentDepthMapValue) break;

    // shift texture coordinates along direction of P
    currentTexCoord -= deltaTexCoord;

    // get depthmap value at current texture coordinates
    currentDepthMapValue = texture(
      u_texturesDisplacement,
      vec3(currentTexCoord, float(displacementLayer))).r;

    // get depth of next layer
    currentLayerDepth += layerDepth;
  }

  // get texture coordinates before collision (reverse operations)
  vec2 prevTexCoord = currentTexCoord + deltaTexCoord;

  // get depth after and before collision for linear interpolation
  float afterDepth  = currentDepthMapValue - currentLayerDepth;
  float beforeDepth = texture(
    u_texturesDisplacement,
    vec3(prevTexCoord, float(displacementLayer))).r
    - currentLayerDepth + layerDepth;

  // interpolation of texture coordinates
  float weight = afterDepth / (afterDepth - beforeDepth);
  vec2 finalTexCoord = prevTexCoord * weight + currentTexCoord * (1.0 - weight);

  return finalTexCoord;
}

#ifdef USE_PARALLAX_MAP_MARCH
// https://www.reddit.com/r/GraphicsProgramming/comments/18qqz77/parallax_occlusion_mapping_revisited/
vec2 parallaxMapMarch (
  const uint materialIndex,
  const vec3 tangentDir,
  const float parallaxDepth,
  const vec2 texCoord)
{
  int displacementLayer = int(u_materials[materialIndex].displacementMapTex.x);

  vec2 pomUV = texCoord, optimalUV = texCoord;

  const ivec2 texSize = textureSize(u_texturesDisplacement, 0).xy;
  vec2 tanSpaceMarchDir = normalize(tangentDir.xy) / length(texSize);

  // Smaller samples at oblique angles
  tanSpaceMarchDir *= abs(normalize(tangentDir).z);

  float marchLen = length(tanSpaceMarchDir);
  float ratio = tangentDir.z / (length(tangentDir.xy) * parallaxDepth);
  float queryHeight = 0.0, calcHeight = 0.0;

  for (uint i = 0; i <= 40; i++)
  {
    pomUV += tanSpaceMarchDir;

    queryHeight = texture(
      u_texturesDisplacement,
      vec3(pomUV, float(diplacementLayer))).r;

    calcHeight = float(i) * marchLen * ratio;

    if (queryHeight >= calcHeight)
      optimalUV = pomUV;

    if (calcHeight >= 1.0) break;
  }
  return optimalUV;
}
#endif

#endif
#endif
