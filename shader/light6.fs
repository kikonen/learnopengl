#version 330 core
struct Material {
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  float shininess;
};
struct Light {
  vec3 pos;
  vec3 dir;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;

  float constant;
  float linear;
  float quadratic;

  bool use;
  bool directional;
  bool point;
};

flat in float texIndex;
in vec3 fragPos;
in vec3 normal;

layout (std140) uniform Data {
  //vec3 viewPos;
  float a;
};
uniform vec3 viewPos;
uniform samplerCube skybox;

uniform Material materials[16];
uniform Light light;

out vec4 fragColor;

void main() {
  int texId = int(texIndex);
  Material material = materials[texId];

  // combined
  //vec3 shaded = light.ambient + light.diffuse + light.specular;
  vec4 texColor = material.diffuse; //light.specular;

  if (texColor.a < 0.1)
    discard;

  fragColor = texColor;
}
