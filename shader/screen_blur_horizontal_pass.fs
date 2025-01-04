#version 460 core

in VS_OUT {
  vec2 texCoords[9];
} fs_in;

layout(binding = UNIT_SOURCE) uniform sampler2D u_sourceTex;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  o_fragColor = vec4(0.0);
  o_fragColor += texture(u_sourceTex, fs_in.texCoords[0]) * 0.02853226260337099;
  o_fragColor += texture(u_sourceTex, fs_in.texCoords[1]) * 0.06723453549491201;
  o_fragColor += texture(u_sourceTex, fs_in.texCoords[2]) * 0.1240093299792275;
  o_fragColor += texture(u_sourceTex, fs_in.texCoords[3]) * 0.1790438646174162;
  o_fragColor += texture(u_sourceTex, fs_in.texCoords[4]) * 0.2023600146101466;
  o_fragColor += texture(u_sourceTex, fs_in.texCoords[5]) * 0.1790438646174162;
  o_fragColor += texture(u_sourceTex, fs_in.texCoords[6]) * 0.1240093299792275;
  o_fragColor += texture(u_sourceTex, fs_in.texCoords[7]) * 0.06723453549491201;
  o_fragColor += texture(u_sourceTex, fs_in.texCoords[8]) * 0.02853226260337099;

  if (o_fragColor.r == 0 && o_fragColor.g == 0 && o_fragColor.b == 0) {
    o_fragColor.a = 0;
  }
}
