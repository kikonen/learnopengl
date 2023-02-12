#version 460 core
in vec3 texCoord;

layout(binding = UNIT_SKYBOX) uniform samplerCube u_skybox;

out vec4 fragColor;

precision mediump float;

void main() {
  fragColor = texture(u_skybox, texCoord);
  //fragColor = vec4(1.0, 0, 0, 1.0);
}