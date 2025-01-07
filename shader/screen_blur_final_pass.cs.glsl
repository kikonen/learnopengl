#version 460 core

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(location = UNIFORM_VIEWPORT) uniform vec2 u_viewport;

layout(rgba16f, binding = UNIT_0) uniform image2D u_destinationTex;

// layout(binding = UNIT_SOURCE) uniform sampler2D u_sourceTex;

layout(binding = UNIT_CHANNEL_0) uniform sampler2D u_channel0;
layout(binding = UNIT_CHANNEL_1) uniform sampler2D u_channel1;
layout(binding = UNIT_CHANNEL_2) uniform sampler2D u_channel2;
// layout(binding = UNIT_CHANNEL_3) uniform sampler2D u_channel3;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  ivec2 texel = ivec2(gl_GlobalInvocationID.xy);

  // NOTE KI no processing outside of viewport
  if (texel.x > u_viewport.x) return;
  if (texel.y > u_viewport.y) return;

  vec2 uv = vec2(texel.x / u_viewport.x, texel.y / u_viewport.y);

  vec4 orig = imageLoad(u_destinationTex, texel);
  vec3 color = orig.rgb;
  color += textureLod(u_channel0, uv, 0).rgb;
  color += textureLod(u_channel1, uv, 0).rgb;
  color += textureLod(u_channel2, uv, 0).rgb;
  // color += textureLod(u_channel3, uv, 0).rgb;
  // color = textureLod(u_sourceTex, uv, 0).rgb * vec3(2, 2, 2);

  imageStore(u_destinationTex, texel, vec4(color, orig.a));
  // imageStore(u_destinationTex, texCoord, vec4(uv.x, 0, 0, 1));
}
