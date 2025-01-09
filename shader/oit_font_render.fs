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

  vec2 atlasCoord;
  flat uvec2 atlasHandle;

  flat uint materialIndex;
  flat uint flags;
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

  // Font shaping
  vec3 color;
  vec3 rgb;
  float alpha;
  {
    sampler2D atlas = sampler2D(fs_in.atlasHandle);

    float dist  = 1.0 - textureLod(atlas, fs_in.atlasCoord.st, 0).r;
    float width = 0.1;
    float edge = 0.9;

    float glyphAlpha = 1.0 - smoothstep(width, width + edge, dist);

    vec3 glyphColor = material.diffuse.rgb;

    // rgb = mix(glow_color, glyphColor, glyphAlpha);
    // float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
    // rgb = glyphColor;

    // vec4 color = vec4(rgb, max(glyphAlpha, mu));

    vec3 borderColor  = vec3(1.0, 0.0, 0.0);
    vec2 borderOffset = vec2(0, 0);
    float borderWidth = 0.2;
    float borderEdge = 0.4;

    float dist2  = 1.0 - textureLod(atlas, fs_in.atlasCoord.st + borderOffset, 0).r;

    float borderAlpha = 1.0 - smoothstep(borderWidth, borderWidth + borderEdge, dist2);

    alpha = glyphAlpha + (1.0 - glyphAlpha) * borderAlpha;
    color = mix(borderColor, glyphColor, glyphAlpha / alpha);
  }

  if (alpha < OIT_MIN_BLEND_THRESHOLD || alpha >= OIT_MAX_BLEND_THRESHOLD)
    discard;

  float weight = clamp(pow(min(1.0, alpha * 10.0) + 0.01, 3.0) * 1e8 * pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

  o_accum = vec4(color.rgb * alpha, alpha) * weight;
  o_reveal = alpha;

  o_fragEmission = material.emission;
}
