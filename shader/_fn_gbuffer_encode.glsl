// Utility functions for gbuffer

// NOTE KI GL_RGB16F
// => *MUST* match FrameBufferAttachment::getGBufferNormal
vec3 encodeGNormal(in vec3 normal) {
  //return normal * 0.5 + 0.5;
  return normal;
}
