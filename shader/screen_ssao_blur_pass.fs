#version 460 core

#include uniform_matrices.glsl
#include uniform_camera.glsl
#include uniform_data.glsl

// NOTE KI depth is *not* used
// => for *stencil test
layout(early_fragment_tests) in;

#include screen_tri_vertex_out.glsl

layout(binding = UNIT_SSAO) uniform sampler2D u_ssaoTex;

layout (location = 0) out float o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  #include screen_tri_tex_coord.glsl

  const vec2 texelSize = 1.0 / vec2(textureSize(u_ssaoTex, 0));
  float result = 0.0;
  for (int x = -2; x < 2; ++x)
  {
    for (int y = -2; y < 2; ++y)
    {
      const vec2 offset = vec2(float(x), float(y)) * texelSize;
      result += texture(u_ssaoTex, texCoord + offset).r;
    }
  }

  o_fragColor = result / (4.0 * 4.0);
}
