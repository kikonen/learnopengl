// Utility functions for gbuffer

// NOTE KI GL_RGB16F
// => *MUST* match FrameBufferAttachment::getGBufferNormal
vec3 encodeGNormal(in vec3 normal) {
  //return normal * 0.5 + 0.5;
  return normal;
}

// https://aras-p.info/texts/CompactNormalStorage.html
// https://gamedev.net/forums/topic/700478-g-buffer-2-channel-normal/5397776/
vec2 encodeGNormalVec2(in vec3 n, in vec3 view)
{
  float f = sqrt(8 * n.z + 8);
  return n.xy / f + 0.5;
}
