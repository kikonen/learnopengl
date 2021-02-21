#version 430 core

#include struct_material.glsl
#include uniform_data.glsl
#include uniform_materials.glsl

flat in int materialIndex;
in vec3 fragPos;
in vec3 normal;

out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  Material material = materials[materialIndex];

  // combined
  vec4 texColor = material.diffuse;

  if (texColor.a < 0.1)
    discard;

  fragColor = texColor;
}
