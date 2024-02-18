#version 460 core

#include uniform_data.glsl

in VS_OUT {
  vec2 texCoord;
  vec3 viewPos;

  flat vec4 diffuse;
  flat uvec2 diffuseTex;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include fn_calculate_fog.glsl

void main() {
  vec4 texColor = fs_in.diffuse;

  if (fs_in.diffuseTex.x > 0) {
    const vec2 texCoord = fs_in.texCoord;
    texColor *= texture(
      sampler2D(fs_in.diffuseTex),
      texCoord);
  }

#ifdef USE_ALPHA
  if (texColor.a < 0.01)
    discard;
#endif

//  texColor.a = clamp(texColor.a, 0, 0.5);

#ifndef USE_BLEND
  texColor = vec4(texColor.rgb, 1.0);
#endif

  texColor = calculateFog(fs_in.viewPos, texColor);

  o_fragColor = texColor;
//  o_fragColor = vec4(1, 0, 0, 1);
}
