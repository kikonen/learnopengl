#version 450 core
layout (location = 0) in vec4 a_pos;
layout (location = 1) in vec2 a_texCoord;

out vec2 texCoord;

void main()
{
  texCoord = a_texCoord;
  gl_Position = a_pos;
}
