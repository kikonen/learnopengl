#version 330 core
in vec2 texCoord;
in vec4 color;

uniform sampler2D texture1;
//uniform sampler2D texture2;
uniform vec3 lightColor;

out vec4 fragColor;

void main()
{
//  FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
  fragColor = texture(texture1, texCoord);
}
