#version 460 core

#include struct_material.glsl

#include ssbo_materials.glsl
#include uniform_textures.glsl

in GS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
  flat uint highlightIndex;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main() {
  const vec2 texCoord = fs_in.texCoord;
  #include var_tex_material_alpha.glsl

  if (alpha < 0.6)
    discard;

  Material highlightMaterial = u_materials[fs_in.highlightIndex];

  o_fragColor = highlightMaterial.diffuse;
}
