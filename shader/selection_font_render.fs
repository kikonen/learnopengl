#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include ssbo_materials.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  vec2 atlasCoord;
  flat uvec2 atlasHandle;
  flat uint highlightIndex;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include fn_shape_font.glsl

void main()
{
  vec4 color;
  shapeFont(fs_in.atlasHandle, fs_in.atlasCoord, color);

#ifdef USE_ALPHA
#ifdef USE_BLEND
  if (color.a < OIT_MAX_BLEND_THRESHOLD)
    discard;
#else
  if (color.a < GBUFFER_ALPHA_THRESHOLD)
    discard;
#endif
#endif

  const uint materialIndex = fs_in.highlightIndex;
  o_fragColor = u_materials[materialIndex].diffuse;
}
