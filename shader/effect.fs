#version 460 core

#include include/uniform_data.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  flat uint entityIndex;

  vec4 color;
  vec2 texCoord;
  vec3 worldPos;
  vec3 normal;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

#include include/fn_effect_explosion.glsl

void main() {

  vec4 texColor = rayExplosion(worldPos);

#ifdef USE_ALPHA
  if (texColor.a < 0.1)
    discard;
#endif

  o_fragColor = texColor;
}
