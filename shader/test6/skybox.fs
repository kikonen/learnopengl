#version 450 core
in vec3 texCoords;

uniform samplerCube u_skybox;

out vec4 fragColor;

void main() {
  fragColor = texture(u_skybox, texCoords);
  //fragColor = vec4(1.0, 0, 0, 1.0);
}
