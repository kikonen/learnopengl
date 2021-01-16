#version 330 core
struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

flat in float texIndex;
in vec3 fragPos;
in vec3 normal;

layout (std140) uniform Data {
  vec3 viewPos;
};

uniform Material materials[16];

out vec4 fragColor;

void main() {
  int texId = int(texIndex);
  Material material = materials[texId];

  // combined
  vec4 texColor = material.diffuse;

  if (texColor.a < 0.1)
    discard;

  fragColor = texColor;
}
