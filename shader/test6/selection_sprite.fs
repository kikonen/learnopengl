#version 450 core

#include constants.glsl

#include struct_material.glsl

#include uniform_materials.glsl

in GS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
} fs_in;

uniform sampler2D u_textures[TEX_COUNT];

layout (location = 0) out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision lowp float;

void main() {
  uint matIdx = fs_in.materialIndex;
  int diffuseTexIdx = u_materials[matIdx].diffuseTex;

  float alpha;
  if (diffuseTexIdx >= 0) {
    alpha = texture(u_textures[diffuseTexIdx], fs_in.texCoord).a;
  } else {
    alpha = u_materials[matIdx].diffuse.a;
  }

  if (alpha < 0.6)
    discard;

  fragColor = vec4(0.8, 0.0, 0.0, 1.0);
}
