// Utility functions for gbuffer

// NOTE KI GL_RGB16F
// => *MUST* match FrameBufferAttachment::getGBufferNormal
// => for some reason DOES NOT WORK in intel GPU
vec3 decodeGNormal(in vec2 texCoord)
{
  //return texture(g_normal, texCoord).xyz * 2.0 - 1.0;
  return textureLod(g_normal, texCoord, 0).xyz;
}

// https://aras-p.info/texts/CompactNormalStorage.html
// https://gamedev.net/forums/topic/700478-g-buffer-2-channel-normal/5397776/
vec3 decodeGNormal2(in vec2 texCoord)
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

// https://github.khronos.org/KTX-Software/ktxtools/ktx_encode.html
vec3 decodeGNormalKtx(in vec2 texCoord)
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
