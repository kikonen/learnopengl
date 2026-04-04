// Utility functions for gbuffer

// NOTE KI GL_RGB16 => thus encode into [0, 1] range
// => *MUST* match FrameBufferAttachment::getGBufferNormal
// => for some reason DOES NOT WORK in intel GPU
vec3 decodeGNormal(const vec2 texCoord)
{
  // return texture(g_normal, texCoord).xyz * 2.0 - 1.0;
  return textureLod(g_normal, texCoord, 0).xyz;
}

// https://aras-p.info/texts/CompactNormalStorage.html
// https://gamedev.net/forums/topic/700478-g-buffer-2-channel-normal/5397776/
vec3 decodeGNormal2(const vec2 texCoord)
{
  // TODO KI does NOT work propearly, corrupted possibly due to sign of Z-dir
  vec2 enc = textureLod(g_normal, texCoord, 0).xy;
  vec2 fenc = enc * 4 - 2;

  float f = dot(fenc, fenc);
  float g = sqrt(1 - f / 4);

  vec3 n;
  n.xy = fenc * g;
  n.z = 1 - f / 2;

  return n;
}


// Octahedral encoding
// Decode normal from RG16_SNORM or RG16F
vec3 decodeGNormal_RG(const vec2 texCoord)
{
  const vec2 enc = textureLod(g_normal, texCoord, 0).xy;
  vec3 n = vec3(enc, 1.0 - abs(enc.x) - abs(enc.y));
  if (n.z < 0.0) {
    n.xy = (1.0 - abs(n.yx)) * signNotZero(n.xy);
  }
  return normalize(n);
}

// https://github.khronos.org/KTX-Software/ktxtools/ktx_encode.html
vec3 decodeGNormalKtx(const vec2 texCoord)
{
  vec3 n;

  // Load in [0, 1]
  n.xy = textureLod(g_normal, texCoord, 0).xy;

  // Unpack to [-1, 1]
  n.xy = n.xy * 2.0 - 1.0;

  // Compute Z
  n.z = sqrt(1 - dot(n.xy, n.xy));

  return n;
}
