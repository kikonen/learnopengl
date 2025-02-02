// Utility functions for gbuffer

// NOTE KI GL_RGB16F
// => *MUST* match FrameBufferAttachment::getGBufferNormal
vec3 encodeGNormal(in vec3 normal) {
  //return normal * 0.5 + 0.5;
  return normal;
}

// https://aras-p.info/texts/CompactNormalStorage.html
// https://gamedev.net/forums/topic/700478-g-buffer-2-channel-normal/5397776/
// https://gamedev.stackexchange.com/questions/203188/problems-with-normal-recovery-from-storage-as-two-floats-and-normal-from-depth-b
vec2 encodeGNormal2(in vec3 normal)
{
  // TODO KI does NOT work propearly, corrupted possibly due to sign of Z-dir
  float f = sqrt(8 * normal.z + 8);
  return normal.xy / f + 0.5;
}
