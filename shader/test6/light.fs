#version 450 core

#include struct_material.glsl
#include uniform_data.glsl
#include uniform_materials.glsl

flat in int materialIndex;
in vec3 fragPos;
in vec3 normal;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 objectID;

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
  objectID = vec4(normal, 1.0);
}
