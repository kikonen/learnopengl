#version 460 core

#include ssbo_materials.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

#ifdef USE_ALPHA
in VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
  flat uint flags;
  flat uint highlightIndex;
} fs_in;
#else
in VS_OUT {
  flat uint highlightIndex;
} fs_in;
#endif

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;


void main() {
#ifdef USE_ALPHA
  {
    const vec2 texCoord = fs_in.texCoord;

    #include var_tex_material_alpha.glsl

    // NOTE KI this works badly for blended objects if threshold too big
    if (alpha < 0.1)
      discard;
  }
#endif

  const uint materialIndex = fs_in.highlightIndex;

  o_fragColor = u_materials[materialIndex].diffuse;
}
