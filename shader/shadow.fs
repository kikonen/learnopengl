#version 460 core

#ifdef USE_ALPHA
#include "include/tbo_materials.glsl"
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


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#ifdef USE_ALPHA
ResolvedMaterial material;

#include "include/fn_fill_material_tbo.glsl"
#endif

void main()
{
#ifdef USE_ALPHA
  {
    const vec2 texCoord = fs_in.texCoord;
    float alpha = readMaterialAlphaTBO(fs_in.materialIndex, fs_in.texCoord);

    // NOtE KI experimental value; depends from few aspects in blended windows
    if (alpha < SHADOW_ALPHA_THRESHOLD)
      discard;
  }
#endif
}
