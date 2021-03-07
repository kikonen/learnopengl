#version 450 core

#include struct_material.glsl
#include uniform_data.glsl
#include uniform_materials.glsl

in VS_OUT {
  flat vec4 objectID;
  vec4 color;
  flat int materialIndex;
  vec2 texCoords;
  vec3 fragPos;
  vec3 normal;
} fs_in;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 fragObjectID;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  Material material = materials[fs_in.materialIndex];

  // combined
  vec4 texColor = material.diffuse;

  if (texColor.a < 0.1)
    discard;

  fragColor = texColor;
  fragObjectID = fs_in.objectID;
}
