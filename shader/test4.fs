#version 330 core
in float colorW;

uniform vec3 modelColor;

out vec4 fragColor;

void main() {
//  fragColor = vec4(max(0.0f, modelColor.x - 0.1f * colorW), max(0.0f, modelColor.y - 0.1f * colorW), max(0.0f, modelColor.z - 0.1f * colorW), 1.0f);
  fragColor = vec4(modelColor.x, modelColor.y, modelColor.z, 1.0f);
}
