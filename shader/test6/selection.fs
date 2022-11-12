#version 450 core

#include constants.glsl

#ifdef USE_ALPHA
#include struct_material.glsl

#include uniform_materials.glsl
#include uniform_textures.glsl
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

//uniform sampler2D u_textures[TEX_COUNT];
#endif

layout (location = 0) out vec4 fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

precision lowp float;

void main() {
#ifdef USE_ALPHA
  uint matIdx = fs_in.materialIndex;
  int diffuseTexIdx = u_materials[matIdx].diffuseTex;

  float alpha;
  if (diffuseTexIdx >= 0) {
    alpha = texture(u_textures[diffuseTexIdx], fs_in.texCoord).a;
  } else {
    alpha = u_materials[matIdx].diffuse.a;
  }

  if (alpha < 0.6)
    discard;
#endif

  fragColor = vec4(0.8, 0.0, 0.0, 1.0);
}
