#version 330 core
in vec3 ourColor;

out vec4 fragColor;

void main() {
  fragColor = vec4(ourColor, 1.0f);
//  fragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
