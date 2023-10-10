#version 460 core

#include struct_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_materials.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  flat uint materialIndex;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

Material material;

void main() {
  material = u_materials[fs_in.materialIndex];

  vec2 pos = gl_PointCoord;

  sampler2D sampler = sampler2D(material.diffuseTex);
  vec4 texColor = texture(sampler, vec2(pos.x, 1.0 - pos.y));

#ifdef USE_ALPHA
  if (texColor.a < 0.1)
    discard;
#endif

  o_fragColor = texColor;
}
