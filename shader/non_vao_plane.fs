#version 460 core

#include "include/ssbo_materials.glsl"

#include "include/uniform_data.glsl"

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  vec2 texCoord;
  flat uint materialIndex;
} fs_in;

layout(location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

void main()
{
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;
  #include "include/apply_parallax.glsl"

  #include "include/var_tex_material.glsl"

  // if (fs_in.vertexID == 0) {
  //   o_fragColor = vec4(1, 0, 0, 1);
  // // } else if (fs_in.vertexID < 2) {
  // //   o_fragColor = vec4(0, 1, 1, 1);
  // } else if (fs_in.vertexID == 2) {
  //   o_fragColor = vec4(0, 1, 1, 1);
  // } else {
  //   o_fragColor = vec4(0, 0, 0, 1);
  // }

  o_fragColor = vec4(material.diffuse.rgb, 1);

  if (gl_FrontFacing) {
    o_fragColor.b = 1;
  }
}
