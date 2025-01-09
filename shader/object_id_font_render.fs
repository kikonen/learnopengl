#version 460 core


#ifdef USE_ALPHA
#include struct_material.glsl
#include struct_resolved_material.glsl

#include ssbo_materials.glsl

in VS_OUT {
  flat vec4 objectID;

  vec2 texCoord;

  vec2 atlasCoord;
  flat uvec2 atlasHandle;

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

ResolvedMaterial material;

#include fn_shape_font.glsl

void main() {
#ifdef USE_ALPHA
  {
    const vec2 texCoord = fs_in.texCoord;
    #include var_tex_material_alpha.glsl

    material.diffuse = vec4(1, 1, 1, 1);

    vec4 color;
    shapeFont(fs_in.atlasHandle, fs_in.atlasCoord, color);
    alpha = color.a;

    // NOtE KI experimental value; depends from few aspects in blended windows
    // NOTE KI this works badly for blended objects if threshold too big
    if (alpha < 0.1)
      discard;
  }
#endif

  o_fragObjectID = fs_in.objectID;
}
