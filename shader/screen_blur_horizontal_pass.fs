#version 460 core

in VS_OUT {
  vec2 texCoords[9];
} fs_in;

layout(binding = UNIT_SOURCE) uniform sampler2D u_sourceTex;

layout (location = 0) out vec3 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  vec3 color = vec3(0.0);
  color += texture(u_sourceTex, fs_in.texCoords[0]).rgb * 0.02853226260337099;
  color += texture(u_sourceTex, fs_in.texCoords[1]).rgb * 0.06723453549491201;
  color += texture(u_sourceTex, fs_in.texCoords[2]).rgb * 0.1240093299792275;
  color += texture(u_sourceTex, fs_in.texCoords[3]).rgb * 0.1790438646174162;
  color += texture(u_sourceTex, fs_in.texCoords[4]).rgb * 0.2023600146101466;
  color += texture(u_sourceTex, fs_in.texCoords[5]).rgb * 0.1790438646174162;
  color += texture(u_sourceTex, fs_in.texCoords[6]).rgb * 0.1240093299792275;
  color += texture(u_sourceTex, fs_in.texCoords[7]).rgb * 0.06723453549491201;
  color += texture(u_sourceTex, fs_in.texCoords[8]).rgb * 0.02853226260337099;

  o_fragColor = color;
}
