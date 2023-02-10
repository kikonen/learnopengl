#version 330 core

uniform vec3 triColor;

out vec4 outColor;

void main()
{
  outColor = vec4(triColor, 1.0);
}
