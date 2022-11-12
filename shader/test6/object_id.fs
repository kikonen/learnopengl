#version 450 core

#include constants.glsl

#ifdef USE_ALPHA
#include struct_material.glsl

#include uniform_materials.glsl
#include uniform_textures.glsl

in VS_OUT {
  flat vec4 objectID;

  vec2 texCoord;
  flat uint materialIndex;
} fs_in;

#else

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  flat vec4 objectID;
} fs_in;
#endif


layout (location = 0) out vec4 fragObjectID;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
#ifdef USE_ALPHA
  #include var_tex_material_alpha.glsl

  // NOtE KI experimental value; depends from few aspects in blended windows
  if (alpha < 0.4)
    discard;
#endif

  fragObjectID = fs_in.objectID;
}
