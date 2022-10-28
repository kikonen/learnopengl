#version 450 core

#include constants.glsl

#include uniform_data.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  vec4 color;
  vec2 texCoords;
  vec3 fragPos;
  vec3 normal;
} fs_in;

layout (location = 0) out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include fn_effect_explosion.glsl

void main() {

  vec4 texColor = rayExplosion(fragPos);

#ifdef USE_ALPHA
  if (texColor.a < 0.1)
    discard;
#endif

  fragColor = texColor;
}
