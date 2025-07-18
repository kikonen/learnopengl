#version 460 core

#include ssbo_materials.glsl

#include uniform_matrices.glsl
#include uniform_camera.glsl

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in VS_OUT {
  vec3 worldPos;
  flat uint materialIndex;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main() {
  const uint materialIndex = fs_in.materialIndex;

  o_fragColor = u_materials[materialIndex].diffuse;
}
