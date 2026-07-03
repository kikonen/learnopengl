#version 460 core

#include "include/tbo_materials.glsl"

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;

in TES_OUT {
  flat uint highlightIndex;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

#include "include/fn_fill_material_tbo.glsl"

void main() {
  const uint materialIndex = fs_in.highlightIndex;
  fillMaterialPlainTBO(materialIndex);

  o_fragColor = material.diffuse;
}
