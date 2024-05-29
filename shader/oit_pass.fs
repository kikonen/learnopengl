#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_data.glsl
#include ssbo_materials.glsl

// NOTE KI depth is *not* updated in OIT pass
// => testing against solid depth
// NOTE KI "early_fragment_tests" cannot be used at same same with alpha
// => disables "discard" logic
//layout(early_fragment_tests) in;

in VS_OUT {
  vec2 texCoord;

  flat uint materialIndex;
  flat uint shapeIndex;
} fs_in;

LAYOUT_OIT_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

ResolvedMaterial material;

void main()
{
  const uint materialIndex = fs_in.materialIndex;

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

  vec4 color = material.diffuse;

  const float alpha = color.a;

  if (alpha < 0.01 || alpha >= 0.95)
    discard;

  float weight = clamp(pow(min(1.0, alpha * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

  o_accum = vec4(color.rgb * alpha, alpha) * weight;
  o_reveal = alpha;
}
