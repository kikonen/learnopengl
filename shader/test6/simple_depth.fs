#version 460 core

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
} fs_in;

#endif

precision mediump float;

void main()
{
#ifdef USE_ALPHA
  #include var_tex_material_alpha.glsl

  // NOtE KI experimental value; depends from few aspects in blended windows
  if (alpha < 0.7)
    discard;
#endif

    // gl_FragDepth = gl_FragCoord.z;
}
