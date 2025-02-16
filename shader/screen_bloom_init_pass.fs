#version 460 core

#include uniform_matrices.glsl
#include uniform_data.glsl

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(binding = UNIT_SOURCE) uniform sampler2D u_sourceTex;
layout(binding = UNIT_G_EMISSION) uniform sampler2D g_emission;		\

layout (location = 0) out vec3 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  const vec2 texCoord = fs_in.texCoord;

  vec4 color = textureLod(u_sourceTex, texCoord, 0);
  vec3 emission = textureLod(g_emission, texCoord, 0).rgb;

  color.rgb += emission.rgb;

  const vec3 T = vec3(0.2126, 0.7152, 0.0722);
  const float brightness = dot(color.rgb, T);

  if (length(emission) > 0) {
    o_fragColor = emission.rgb;
  } else if (brightness > u_bloomThreshold) {
    o_fragColor = color.rgb;
  } else {
    o_fragColor = vec3(0.0);
  }
}
