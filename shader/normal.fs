#version 460 core

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;


in GS_OUT {
  vec4 fragColor;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  o_fragColor = fs_in.fragColor;
}
