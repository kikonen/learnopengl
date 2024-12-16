#version 460 core

#include uniform_data.glsl

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(binding = UNIT_VIEWPORT) uniform sampler2D u_viewportTex;

layout(location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  vec4 color = textureLod(u_viewportTex, fs_in.texCoord, 0);
  o_fragColor = color;
}
