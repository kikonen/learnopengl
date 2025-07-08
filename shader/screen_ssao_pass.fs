#version 460 core

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_debug.glsl
#include uniform_buffer_info.glsl

// NOTE KI depth is *not* used
// => for *stencil test
layout(early_fragment_tests) in;

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(binding = UNIT_G_ALBEDO) uniform sampler2D g_albedo;
layout(binding = UNIT_G_NORMAL) uniform sampler2D g_normal;
layout(binding = UNIT_G_DEPTH) uniform sampler2D g_depth;

layout (location = 0) out float o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

float calculateSsao(
  in vec3 worldPos)
{
  float dist = length(worldPos);

  float distRatio = 4.0 * dist / u_fogEnd;
  float fogFactor = clamp(exp(-distRatio * u_fogDensity * distRatio * u_fogDensity), 0.01, 1.0);

  float alpha = 1.0 - fogFactor;
  // return vec4(u_fogColor.xyz, alpha);
  return clamp(worldPos.z / 50.0, -1, 1) * 0.5 + 0.5;
}

void main()
{
  //const vec2 texCoord = gl_FragCoord.xy / u_bufferResolution;
  const vec2 texCoord = fs_in.texCoord;

  // https://mynameismjp.wordpress.com/2010/09/05/position-from-depth-3/
  // https://ahbejarano.gitbook.io/lwjglgamedev/chapter-19
  vec3 worldPos;
  vec3 viewPos;
  float depth;
  {
    // NOTE KI pixCoord == texCoord in fullscreen quad
    const vec2 pixCoord = texCoord;
    depth = textureLod(g_depth, pixCoord, 0).x;

    const vec4 clip = vec4(
      pixCoord.x * 2.0 - 1.0,
      pixCoord.y * 2.0 - 1.0,
      depth * 2.0 - 1.0,
      1.0);
    vec4 viewW  = u_invProjectionMatrix * clip;
    viewPos  = viewW.xyz / viewW.w;
    worldPos = (u_invViewMatrix * vec4(viewPos, 1)).xyz;
  }

  o_fragColor = calculateSsao(worldPos);
}
