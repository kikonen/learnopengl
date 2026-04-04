#include "include/fn_uniform_camera.glsl"

vec3 getViewPosFromGBuffer(const vec2 texCoord)
{
  // NOTE KI pixCoord == texCoord in fullscreen quad
  const float depth = textureLod(g_depth, texCoord, 0).x;
  return getViewPosFromTexCoord(depth, texCoord);
}

// @return non linearized depth
float getDepthFromGBuffer(const vec2 texCoord)
{
  // NOTE KI pixCoord == texCoord in fullscreen quad
  return textureLod(g_depth, texCoord, 0).x;
}
