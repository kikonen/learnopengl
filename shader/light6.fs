#version 330 core
struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};

flat in float materialIndex;
in vec3 fragPos;
in vec3 normal;

layout (std140) uniform Data {
  vec3 viewPos;
  float time;
};

uniform Material materials[16];

out vec4 fragColor;

void main() {
  int matIdx = int(materialIndex);
  Material material = materials[matIdx];

  // combined
  vec4 texColor = material.diffuse;

  if (texColor.a < 0.1)
    discard;

  fragColor = texColor;
}
