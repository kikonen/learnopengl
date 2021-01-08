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

  if (light.use) {
    // ambient
    vec3 ambient = material.ambient * light.ambient;

    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light.pos - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);

    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    // specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    // combined
    vec3 shaded = ambient + diffuse + specular;

    fragColor = vec4(shaded, 1.0);
  } else {
    fragColor = vec4(light.specular, 1.0);
  }
}
