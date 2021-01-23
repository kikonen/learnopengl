#version 330 core

#include struct_material.glsl
#include uniform_data.glsl
#include uniform_materials.glsl

flat in float materialIndex;
in vec3 fragPos;
in vec3 normal;

out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  int matIdx = int(materialIndex);
  Material material = materials[matIdx];

  // combined
  vec4 texColor = material.diffuse;

  if (texColor.a < 0.1)
    discard;

  fragColor = texColor;
}
