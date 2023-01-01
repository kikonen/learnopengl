#version 460 core

#include constants.glsl

#ifdef USE_ALPHA
#include struct_material.glsl

#include uniform_materials.glsl
#include uniform_textures.glsl
#endif

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

#ifdef USE_ALPHA
in VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
  flat uint highlightIndex;
} fs_in;
#else
in VS_OUT {
  flat uint highlightIndex;
} fs_in;
#endif

layout (location = 0) out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision mediump float;

void main() {
#ifdef USE_ALPHA
  #include var_tex_material_alpha.glsl

  if (alpha < 0.6)
    discard;
#endif

  Material highlightMaterial = u_materials[fs_in.highlightIndex];

  fragColor = highlightMaterial.diffuse;
}
