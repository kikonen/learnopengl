#version 450 core

#include constants.glsl

#include struct_material.glsl

#include uniform_materials.glsl
#include uniform_textures.glsl

uniform vec4 u_highlightColor;

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

  fragColor = u_highlightColor;
}
