#version 330 core
in vec2 texCoord;
in vec4 color;
in vec3 fragPos;
in vec3 normal;

uniform sampler2D texture1;
//uniform sampler2D texture2;

uniform vec3 lightPos;
uniform vec4 lightColor;

out vec4 fragColor;

void main()
{
  vec4 texColor = texture(texture1, texCoord);

  if (texColor.a < 0.1)
    discard;

  fragColor = texColor;
}
