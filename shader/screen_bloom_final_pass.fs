#version 460 core

#include uniform_matrices.glsl
#include uniform_data.glsl

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(binding = UNIT_EFFECT_ALBEDO) uniform sampler2D scene_albedo;
layout(binding = UNIT_EFFECT_WORK) uniform sampler2D effect_bloomBlur;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  const vec2 texCoord = fs_in.texCoord;

  vec3 hdrColor = texture(scene_albedo, texCoord).rgb;
  vec3 bloomColor = texture(effect_bloomBlur, texCoord).rgb;

  hdrColor += bloomColor;

  o_fragColor = vec4(hdrColor, 1.0);
}
