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
  color += textureLod(u_sourceTex, fs_in.texCoords[0], 0).rgb * 0.02853226260337099;
  color += textureLod(u_sourceTex, fs_in.texCoords[1], 0).rgb * 0.06723453549491201;
  color += textureLod(u_sourceTex, fs_in.texCoords[2], 0).rgb * 0.1240093299792275;
  color += textureLod(u_sourceTex, fs_in.texCoords[3], 0).rgb * 0.1790438646174162;
  color += textureLod(u_sourceTex, fs_in.texCoords[4], 0).rgb * 0.2023600146101466;
  color += textureLod(u_sourceTex, fs_in.texCoords[5], 0).rgb * 0.1790438646174162;
  color += textureLod(u_sourceTex, fs_in.texCoords[6], 0).rgb * 0.1240093299792275;
  color += textureLod(u_sourceTex, fs_in.texCoords[7], 0).rgb * 0.06723453549491201;
  color += textureLod(u_sourceTex, fs_in.texCoords[8], 0).rgb * 0.02853226260337099;

  o_fragColor = color;
}
