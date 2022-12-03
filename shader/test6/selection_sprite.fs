#version 450 core

#include constants.glsl

#include struct_material.glsl

#include uniform_materials.glsl
#include uniform_textures.glsl

in GS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
} fs_in;

layout (location = 0) out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

void main() {
  #include var_tex_material_alpha.glsl

  if (alpha < 0.6)
    discard;

  fragColor = vec4(0.8, 0.0, 0.0, 1.0);
}
