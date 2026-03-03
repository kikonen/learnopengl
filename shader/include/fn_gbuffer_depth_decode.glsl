// Full screen effect viewPos from g_depth
//
// https://mynameismjp.wordpress.com/2010/09/05/position-from-depth-3/
// https://ahbejarano.gitbook.io/lwjglgamedev/chapter-19
//
// NOTE KI pixCoord == texCoord in fullscreen quad
vec3 getViewPosFromTexCoord(float depth, const vec2 texCoord)
{
  // clip space [-1, 1]
  const vec4 ndcPos = vec4(
    texCoord.x * 2.0 - 1.0,
    texCoord.y * 2.0 - 1.0,
    depth * 2.0 - 1.0,
    1.0);
  vec4 viewW  = u_invProjectionMatrix * ndcPos;
  return viewW.xyz / viewW.w;
}

// Convert viewPos to worldPos
vec3 getWorldPosFromViewPos(const vec3 viewPos)
{
  return (u_invViewMatrix * vec4(viewPos, 1.0)).xyz;
}

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
