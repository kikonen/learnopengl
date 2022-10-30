#version 450 core
out vec4 fragColor;

in vec4 color;

precision lowp float;

void main()
{
  fragColor = color;
}
