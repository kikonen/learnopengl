// Utility functions for gbuffer

// NOTE KI GL_RGB16 => thus encode into [0, 1] range
// => *MUST* match FrameBufferAttachment::getGBufferNormal
vec3 encodeGNormal(const vec3 normal) {
  // return normalize(normal) * 0.5 + 0.5;
  return normalize(normal);
}

// https://aras-p.info/texts/CompactNormalStorage.html
// https://gamedev.net/forums/topic/700478-g-buffer-2-channel-normal/5397776/
// https://gamedev.stackexchange.com/questions/203188/problems-with-normal-recovery-from-storage-as-two-floats-and-normal-from-depth-b
vec2 encodeGNormal2(const vec3 normal)
{
  // TODO KI does NOT work propearly, corrupted possibly due to sign of Z-dir
  float f = sqrt(8 * normal.z + 8);
  return normal.xy / f + 0.5;
}

// Octahedral encoding
// Store only XY in RG16_SNORM or RG16F
vec2 encodeGNormal_RG(in vec3 n)
{
  n /= abs(n.x) + abs(n.y) + abs(n.z);
  if (n.z < 0.0) {
    n.xy = (1.0 - abs(n.yx)) * signNotZero(n.xy);
  }
  return n.xy;
}
