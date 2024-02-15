#version 460 core

#include struct_material.glsl
#include ssbo_materials.glsl
#include uniform_textures.glsl

in GS_OUT {
  flat vec4 objectID;

  vec2 texCoord;
  flat uint materialIndex;
} fs_in;

layout (location = 0) out vec4 o_fragObjectID;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  const vec2 texCoord = fs_in.texCoord;
  #include var_tex_material_alpha.glsl

  // NOtE KI experimental value; depends from few aspects in blended windows
  if (alpha < 0.4)
    discard;

  o_fragObjectID = fs_in.objectID;
}
