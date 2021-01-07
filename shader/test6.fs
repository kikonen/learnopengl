#version 330 core
in vec3 color;
in vec3 fragPos;
in vec3 normal;

uniform vec3 viewPos;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform bool useLight;

out vec4 fragColor;

void main() {
  if (useLight) {
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    // combined
    vec3 shaded = (ambient + diffuse + specular) * color;

    fragColor = vec4(shaded, 1.0);
  } else {
    fragColor = vec4(color, 1.0);
  }
}
