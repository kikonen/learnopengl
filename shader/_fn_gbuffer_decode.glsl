// Utility functions for gbuffer

// NOTE KI GL_RGB16F
// => *MUST* match FrameBufferAttachment::getGBufferNormal
// => for some reason DOES NOT WORK in intel GPU
vec3 decodeGNormal(in vec2 texCoord) {
  //return texture(g_normal, texCoord).xyz * 2.0 - 1.0;
  return texture(g_normal, texCoord).xyz;
}
