#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include ssbo_materials.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
#ifdef USE_CUBE_MAP
  vec3 worldPos;
#endif
  vec3 viewPos;
  vec3 normal;
  vec2 texCoord;

  vec2 atlasCoord;

  flat uvec2 atlasHandle;
  flat uint materialIndex;
  flat uint flags;

#ifdef USE_TBN
  mat3 tbn;
#endif
#ifdef USE_PARALLAX
  vec3 viewTangentPos;
  vec3 tangentPos;
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;

LAYOUT_G_BUFFER_OUT;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

//vec3 glyph_color    = vec3(1.0, 1.0, 1.0);
const float glyph_center   = 0.5;

// vec3 outline_color  = vec3(1.0, 0.0, 0.0);
// const float outline_center = 1.2;

vec3 glow_color     = vec3(1.0, 1.0, 1.0);
const float glow_center    = 2.25;

ResolvedMaterial material;

#ifdef USE_PARALLAX
#include fn_calculate_parallax_mapping.glsl
#endif
#include fn_gbuffer_encode.glsl

void main()
{
  const uint materialIndex = fs_in.materialIndex;

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

#ifdef USE_ALPHA
#ifdef USE_BLEND
  if (material.diffuse.a < OIT_MAX_BLEND_THRESHOLD)
    discard;
#else
  if (material.diffuse.a < GBUFFER_ALPHA_THRESHOLD)
    discard;
#endif
#endif

  #include var_tex_material_normal.glsl

  if (!gl_FrontFacing) {
    normal = -normal;
  }

#ifdef USE_CUBE_MAP
  const vec3 viewDir = normalize(u_cameraPos - fs_in.worldPos);
  #include var_calculate_cube_map_diffuse.glsl
#endif

  vec4 materialColor = material.diffuse;
  clamp_color(materialColor);

  // Font shaping
  vec3 rgb;
  {
    sampler2D atlas = sampler2D(fs_in.atlasHandle);

    float dist  = 1.0 - textureLod(atlas, fs_in.atlasCoord.st, 0).r;
    float width = 0.1;
    float edge = 0.9;

    float glyphAlpha = 1.0 - smoothstep(width, width + edge, dist);

    vec3 glyphColor = materialColor.rgb;

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

    float alpha = glyphAlpha + (1.0 - glyphAlpha) * glyphAlpha;
    rgb = mix(borderColor, glyphColor, glyphAlpha / alpha);

#ifdef USE_ALPHA
#ifdef USE_BLEND
  if (alpha < OIT_MAX_BLEND_THRESHOLD) {
      discard;
  }
#else
  if (alpha < GBUFFER_ALPHA_THRESHOLD)
    discard;
#endif
#endif
  }
  // rgb = materialColor.rgb;

  o_fragColor = rgb;
  o_fragMetal = material.metal;
  o_fragEmission = material.emission;
  o_fragNormal = encodeGNormal(normal, fs_in.viewPos);
}
