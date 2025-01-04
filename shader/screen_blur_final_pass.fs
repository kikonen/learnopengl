#version 460 core

#include uniform_matrices.glsl
#include uniform_data.glsl

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(binding = UNIT_SOURCE) uniform sampler2D u_sourceTex;

layout(binding = UNIT_CHANNEL_0) uniform sampler2D u_channel0;
layout(binding = UNIT_CHANNEL_1) uniform sampler2D u_channel1;
layout(binding = UNIT_CHANNEL_2) uniform sampler2D u_channel2;
layout(binding = UNIT_CHANNEL_3) uniform sampler2D u_channel3;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  // vec4 orig = texture(u_sourceTex, fs_in.texCoord);
  // // o_fragColor = vec4(orig.rgb, 0.8);
  // o_fragColor = orig;// * vec4(5, 0, 5, 1);
  // if (orig.r > 0 || orig.g > 0 || orig.b > 0) {
  //   o_fragColor.a = 1;
  // } else {
  //   o_fragColor.a = 0;
  // }

  // o_fragColor = orig * vec4(52, 52, 52, 1);

  vec3 color = vec3(0);
  vec4 src = texture(u_sourceTex, fs_in.texCoord);
  color += texture(u_channel0, fs_in.texCoord).rgb;
  color += texture(u_channel1, fs_in.texCoord).rgb;
  color += texture(u_channel2, fs_in.texCoord).rgb;
  color += texture(u_channel3, fs_in.texCoord).rgb;
  o_fragColor = vec4(color, 1);
  o_fragColor *= vec4(4, 4, 4, 0.25 + 0.5 * sin(u_time * 2.5) * 0.5);
  // o_fragColor = vec4(src.rgb, 0.7);
}
