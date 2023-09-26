#version 460 core

#include uniform_matrices.glsl
#include uniform_data.glsl
//#include uniform_buffer_info.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(binding = UNIT_G_DEPTH) uniform sampler2D g_depth;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

vec4 calculateFog(
  in vec3 viewPos)
{
  float dist = length(viewPos);

  float distRatio = 4.0 * dist / u_fogEnd;
  float fogFactor = clamp(exp(-distRatio * u_fogDensity * distRatio * u_fogDensity), 0.01, 1.0);

  float alpha = 1.0 - fogFactor;
  return vec4(u_fogColor.xyz, alpha);
}

void main()
{
  //const vec2 texCoord = gl_FragCoord.xy / u_bufferResolution;
  const vec2 texCoord = fs_in.texCoord;

  // https://ahbejarano.gitbook.io/lwjglgamedev/chapter-19
  vec3 viewPos;
  {
    float depth = texture(g_depth, texCoord).x * 2.0 - 1.0;

    // NOTE KI no fog over skybox
    if (depth >= 1.0) {
      discard;
    }

    vec4 clip = vec4(texCoord.x * 2.0 - 1.0, texCoord.y * 2.0 - 1.0, depth, 1.0);
    vec4 viewW  = u_invProjectionMatrix * clip;
    viewPos  = viewW.xyz / viewW.w;
  }

  o_fragColor = calculateFog(viewPos);
}
