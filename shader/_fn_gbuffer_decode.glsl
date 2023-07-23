// Utility functions for gbuffer

// NOTE KI GL_RGB16F
// => *MUST* match FrameBufferAttachment::getGBufferNormal
vec3 decodeGNormal(in vec2 texCoord) {
  //return normalize(texture(g_normal, texCoord).xyz * 2.0 - 1.0);
  return texture(g_normal, texCoord).xyz;
}
