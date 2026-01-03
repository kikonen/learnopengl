#version 460 core

#include "include/uniform_data.glsl"

#include "include/screen_tri_vertex_out.glsl"

layout(binding = UNIT_EFFECT_ALBEDO) uniform sampler2D effect_albedo;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  #include "include/screen_tri_tex_coord.glsl"

  vec3 hdrColor = texture(effect_albedo, texCoord).rgb;

  // reinhard
  // vec3 result = hdrColor / (hdrColor + vec3(1.0));
  // exposure
  vec3 result = vec3(1.0) - exp(-hdrColor * u_hdrExposure);

  // also gamma correct while we're at it
  result = pow(result, vec3(1.0 / u_hdrGamma));

  o_fragColor = vec4(result, 1.0);
}
