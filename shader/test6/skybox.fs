#version 430 core
in vec3 texCoords;

uniform samplerCube skybox;

out vec4 fragColor;

void main() {
  fragColor = texture(skybox, texCoords);
  //fragColor = vec4(1.0, 0, 0, 1.0);
}
