#version 460 core

#include "include/ssbo_materials.glsl"

#include "include/uniform_data.glsl"

// NOTE KI depth is *not* updated in OIT pass
// => testing against solid depth
// NOTE KI "early_fragment_tests" cannot be used at same same with alpha
// => disables "discard" logic
//layout(early_fragment_tests) in;

in VS_OUT {
  vec2 texCoord;

  flat uint materialIndex;
  flat uint flags;
} fs_in;

LAYOUT_OIT_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

#include "include/fn_oit_util.glsl"

ResolvedMaterial material;

void main()
{
  const uint materialIndex = fs_in.materialIndex;

  vec2 texCoord = fs_in.texCoord;
  #include "include/apply_parallax.glsl"

  #include "include/var_tex_material.glsl"

  vec4 color = material.diffuse;

  const float alpha = color.a;

  OIT_DISCARD(alpha);

  float weight = clamp(pow(min(1.0, alpha * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

  o_accum = vec4(color.rgb * alpha, alpha) * weight;
  o_reveal = alpha;

  o_fragEmission = material.emission;
}
