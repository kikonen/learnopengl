#version 330 core

#define MAT_COUNT 8

#include struct_material.glsl

flat in float materialIndex;
in vec3 fragPos;
in vec3 normal;

layout (std140) uniform Data {
  vec3 viewPos;
  float time;
};

layout (std140) uniform Materials {
  Material materials[MAT_COUNT];
};

out vec4 fragColor;

void main() {
  int matIdx = int(materialIndex);
  Material material = materials[matIdx];

  // combined
  vec4 texColor = material.diffuse;

  if (texColor.a < 0.1)
    discard;

  fragColor = texColor;
}
