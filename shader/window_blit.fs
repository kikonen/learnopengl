#version 460 core

#include uniform_data.glsl

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(binding = UNIT_VIEWPORT) uniform sampler2D u_viewportTex;

layout(location = UNIFORM_TONE_HDRI) uniform bool u_toneHdri;
layout(location = UNIFORM_GAMMA_CORRECT) uniform bool u_gammaCorrect;

layout(location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  vec4 orig = textureLod(u_viewportTex, fs_in.texCoord, 0);
  vec3 color = orig.rgb;

  if (u_toneHdri) {
    // reinhard
    // vec3 result = hdrColor / (hdrColor + vec3(1.0));
    // exposure
    color = vec3(1.0) - exp(-color * u_hdrExposure);
  }

  if (u_gammaCorrect) {
    // also gamma correct while we're at it
    color = pow(color, vec3(1.0 / u_hdrGamma));
  }

  o_fragColor = vec4(color, orig.a);
}
