#version 460 core

layout (local_size_x = 16, local_size_y = 14, local_size_z = 1) in;

#include uniform_data.glsl
#include uniform_buffer_info.glsl

layout(rgba16f, binding = UNIT_0) uniform image2D u_destinationTex;

// layout(binding = UNIT_SOURCE) uniform sampler2D u_sourceTex;

layout(binding = UNIT_CHANNEL_0) uniform sampler2D u_channel0;
layout(binding = UNIT_CHANNEL_1) uniform sampler2D u_channel1;
layout(binding = UNIT_CHANNEL_2) uniform sampler2D u_channel2;
layout(binding = UNIT_CHANNEL_3) uniform sampler2D u_channel3;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  ivec2 texel = ivec2(gl_GlobalInvocationID.xy);
  vec2 viewport = u_bufferResolution;
  vec2 uv = vec2(texel.x / viewport.x, texel.y / viewport.y);

  vec4 orig = imageLoad(u_destinationTex, texel);
  vec3 color = orig.rgb;
  color += texture(u_channel0, uv).rgb;
  color += texture(u_channel1, uv).rgb;
  color += texture(u_channel2, uv).rgb;
  color += texture(u_channel3, uv).rgb;
  // color = texture(u_sourceTex, uv).rgb * vec3(2, 2, 2);

  imageStore(u_destinationTex, texel, vec4(color, orig.a));
  // imageStore(u_destinationTex, texCoord, vec4(uv.x, 0, 0, 1));
}
