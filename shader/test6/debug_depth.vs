#version 460 core

layout (location = ATTR_POS) in vec4 a_pos;
layout (location = ATTR_TEX) in vec2 a_texCoord;

out vec2 texCoord;

void main()
{
  texCoord = a_texCoord;
  gl_Position = a_pos;
}
