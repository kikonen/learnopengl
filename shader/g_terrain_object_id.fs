#version 460 core

#include include/ssbo_materials.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in TES_OUT {
  flat vec4 objectID;
} fs_in;

layout (location = 0) out vec4 o_fragObjectID;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main() {
  o_fragObjectID = fs_in.objectID;
}
