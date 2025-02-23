#version 460 core

#include uniform_data.glsl

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(binding = UNIT_VIEWPORT) uniform sampler2D u_viewportTex;

layout(location = UNIFORM_GAMMA_CORRECT_ENABLED) uniform bool u_gammaCorrectEnabled;
layout(location = UNIFORM_HDR_TONE_ENABLED) uniform bool u_hdrToneEnabled;
layout(location = UNIFORM_HDR_EXPOSURE_ENABLED) uniform bool u_hdrExposureEnabled;

layout(location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  vec4 orig = textureLod(u_viewportTex, fs_in.texCoord, 0);
  vec3 color = orig.rgb;

  if (u_hdrToneEnabled) {
    color = vec3(1.0) - exp(-color * u_hdrExposure);
  }

  if (u_gammaCorrectEnabled) {
    color = pow(color, vec3(1.0 / u_gammaCorrect));
  }

  o_fragColor = vec4(color, orig.a);
}
