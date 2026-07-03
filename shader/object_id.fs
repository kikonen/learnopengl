#version 460 core


#ifdef USE_ALPHA
#include "include/tbo_materials.glsl"

in VS_OUT {
  flat vec4 objectID;

  vec2 texCoord;
  flat uint materialIndex;
  flat uint flags;
} fs_in;

#else

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  flat vec4 objectID;
} fs_in;
#endif


layout (location = 0) out vec4 o_fragObjectID;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#ifdef USE_ALPHA
ResolvedMaterial material;

#include "include/fn_fill_material_tbo.glsl"
#endif

void main() {
#ifdef USE_ALPHA
  {
    const vec2 texCoord = fs_in.texCoord;
    float alpha = readMaterialAlphaTBO(fs_in.materialIndex, fs_in.texCoord);

    // NOtE KI experimental value; depends from few aspects in blended windows
    // NOTE KI this works badly for blended objects if threshold too big
    if (alpha < 0.1)
      discard;
  }
#endif

  o_fragObjectID = fs_in.objectID;
}
