#include "include/fn_util.glsl"

float linearizeDepthFromUniform(float depth)
{
  return linearizeDepth(depth, u_nearPlane, u_farPlane);
}

// Convert viewPos to worldPos
vec3 getWorldPosFromViewPos(const vec3 viewPos)
{
  return (u_invViewMatrix * vec4(viewPos, 1.0)).xyz;
}

// @return view dir in world
vec3 getWorldDirFromViewDir(const vec3 viewDir)
{
  // Using mat3(u_invViewMatrix) strips translation, which is correct
  // for normals/directions and slightly cheaper than the full vec4 multiply.
  return normalize(mat3(u_invViewMatrix) * viewDir);
}

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
    texCoord * 2.0 - 1.0,
    depth * 2.0 - 1.0,
    1.0);

  vec4 viewW  = u_invProjectionMatrix * ndcPos;
  return viewW.xyz / viewW.w;
}
