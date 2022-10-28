#version 450 core

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

uniform sampler2D textures[TEX_COUNT];
#endif

out vec4 fragColor;

void main() {
#ifdef USE_ALPHA
  int matIdx = fs_in.materialIndex;
  int diffuseTexIdx = materials[matIdx].diffuseTex;

  float alpha;
  if (diffuseTexIdx >= 0) {
    alpha = texture(textures[diffuseTexIdx], fs_in.texCoords).a;
  } else {
    alpha = materials[matIdx].diffuse.a;
  }

  if (alpha < 0.6)
    discard;
#endif

  fragColor = vec4(0.8, 0.0, 0.0, 1.0);
}
