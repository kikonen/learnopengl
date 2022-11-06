#version 450 core
in vec3 texCoord;

uniform samplerCube u_skybox;

out vec4 fragColor;

precision lowp float;

void main() {
  fragColor = texture(u_skybox, texCoord);
  //fragColor = vec4(1.0, 0, 0, 1.0);
}
