#version 330 core
struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};
struct Light {
  vec3 pos;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  bool use;
};

flat in float texIndex;
in vec3 fragPos;
in vec3 normal;

uniform vec3 viewPos;

uniform Material materials[32];
uniform Light light;

out vec4 fragColor;

void main() {
  int texId = int(texIndex);
  Material material = materials[texId];

  // combined
  vec3 shaded = light.ambient + light.diffuse + light.specular;

  fragColor = vec4(shaded, 1.0);
}
