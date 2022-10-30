#version 450 core

#include constants.glsl

#ifdef USE_ALPHA
#include struct_material.glsl

#include uniform_materials.glsl
#endif

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

#ifdef USE_ALPHA
in VS_OUT {
  vec2 texCoords;
  flat int materialIndex;
} fs_in;

uniform sampler2D u_textures[TEX_COUNT];
#endif

precision lowp float;

void main()
{
#ifdef USE_ALPHA
  int matIdx = fs_in.materialIndex;
  int diffuseTexIdx = materials[matIdx].diffuseTex;

  float alpha;
  if (diffuseTexIdx >= 0) {
    alpha = texture(u_textures[diffuseTexIdx], fs_in.texCoords).a;
  } else {
    alpha = materials[matIdx].diffuse.a;
  }

  // NOtE KI experimental value; depends from few aspects in blended windows
  if (alpha < 0.7)
    discard;
#endif

    // gl_FragDepth = gl_FragCoord.z;
}
