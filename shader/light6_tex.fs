#version 330 core
in vec2 texCoord;
in vec3 color;
in vec3 fragPos;
in vec3 normal;

uniform sampler2D texture1;
//uniform sampler2D texture2;

uniform vec3 lightPos;
uniform vec3 lightColor;

out vec4 fragColor;

void main()
{
  fragColor = texture(texture1, texCoord);
}
