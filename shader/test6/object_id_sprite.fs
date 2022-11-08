#version 450 core

#include constants.glsl

#include struct_material.glsl
#include uniform_materials.glsl

in GS_OUT {
  flat vec4 objectID;

  vec2 texCoord;
  flat uint materialIndex;
} fs_in;

uniform sampler2D u_textures[TEX_COUNT];

layout (location = 0) out vec4 fragObjectID;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  uint matIdx = fs_in.materialIndex;
  int diffuseTexIdx = u_materials[matIdx].diffuseTex;

  float alpha;
  if (diffuseTexIdx >= 0) {
    alpha = texture(u_textures[diffuseTexIdx], fs_in.texCoord).a;
  } else {
    alpha = u_materials[matIdx].diffuse.a;
  }

  // NOtE KI experimental value; depends from few aspects in blended windows
  if (alpha < 0.4)
    discard;

  fragObjectID = fs_in.objectID;
}
