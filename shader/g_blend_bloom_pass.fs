#version 460 core

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_buffer_info.glsl

layout(binding = UNIT_EFFECT_ALBEDO) uniform sampler2D effect_albedo;
layout(binding = UNIT_EFFECT_WORK) uniform sampler2D effect_work;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  const vec2 texCoord = gl_FragCoord.xy / u_bufferResolution;

  const float gamma = 2.2;

  vec3 hdrColor = texture(effect_albedo, texCoord).rgb;
  vec3 bloomColor = texture(effect_work, texCoord).rgb;

  vec3 color = hdrColor;

  if (bloomColor.r + bloomColor.g + bloomColor.b > 0.01) {
    // additive blending
    hdrColor += bloomColor;

    // tone mapping
    float exposure = u_effectBloomExposure + sin(u_time * 2) * 0.3;
    color = vec3(1.0) - exp(-hdrColor * exposure);

    // also gamma correct while we're at it
    color = pow(color, vec3(1.0 / gamma));
  }

  o_fragColor = vec4(color, 1.0);
}
