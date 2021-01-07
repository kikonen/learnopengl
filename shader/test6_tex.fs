#version 330 core
in vec3 color;
flat in float texIndex;
in vec2 texCoord;
in vec3 fragPos;
in vec3 normal;

uniform sampler2D textures[8];

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform bool useLight;

out vec4 fragColor;

void main()
{
  int texId = int(texIndex);
  if (useLight) {
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // combined
    vec3 shaded = (ambient + diffuse);

    //  FragColor = mix(texture(textures[texId], TexCoord), texture(texture2, TexCoord), 0.2);
    fragColor = texture(textures[texId], texCoord);// * vec4(color.x, color.y, color.z, 0.1);

    fragColor = texture(textures[texId], texCoord) * vec4(shaded, 1.0);
  } else {
    fragColor = texture(textures[texId], texCoord);
  }
}
