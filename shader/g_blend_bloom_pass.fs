#version 460 core

#include uniform_matrices.glsl
#include uniform_data.glsl
//#include uniform_buffer_info.glsl

// NOTE KI depth is *not* used
// => for *stencil test
layout(early_fragment_tests) in;

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(binding = UNIT_EFFECT_ALBEDO) uniform sampler2D effect_albedo;
layout(binding = UNIT_EFFECT_WORK) uniform sampler2D effect_work;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  //const vec2 texCoord = gl_FragCoord.xy / u_bufferResolution;
  const vec2 texCoord = fs_in.texCoord;

  vec3 hdrColor = texture(effect_albedo, texCoord).rgb;
  vec3 bloomColor = texture(effect_work, texCoord).rgb;

  vec3 color = hdrColor;

  if (bloomColor.r + bloomColor.g + bloomColor.b > 0.01) {
    // additive blending
    hdrColor += bloomColor;

    color = hdrColor + hdrColor * (1.0 + sin(u_time * 2.4)) * 0.2;
  }

  o_fragColor = vec4(color, 1.0);
}
