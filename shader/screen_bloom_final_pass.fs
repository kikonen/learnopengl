#version 460 core

#include uniform_matrices.glsl
#include uniform_camera.glsl
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

  vec4 orig = textureLod(scene_albedo, texCoord, 0);
  vec3 hdrColor = orig.rgb;
  vec3 bloomColor = textureLod(effect_bloomBlur, texCoord, 0).rgb;

  hdrColor += bloomColor;

  o_fragColor = vec4(hdrColor, orig.a);
}
