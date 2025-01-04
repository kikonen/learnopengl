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
const float glyph_center   = 0.50;

vec3 outline_color  = vec3(0.0, 0.0, 0.0);
const float outline_center = 0.55;

vec3 glow_color     = vec3(1.0, 1.0, 1.0);
const float glow_center    = 1.25;

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
    vec4  atlasValue = texture2D(atlas, fs_in.atlasCoord.st);

    float dist  = atlasValue.r;
    float width = fwidth(dist);
    float alpha = smoothstep(glyph_center-width, glyph_center+width, dist);

    vec3 glyph_color = materialColor.rgb;

    rgb = mix(glow_color, glyph_color, alpha);
    float mu = smoothstep(glyph_center, glow_center, sqrt(dist));

    vec4 color = vec4(rgb, max(alpha,mu));
    float beta = smoothstep(outline_center-width, outline_center+width, dist);
    rgb = mix(outline_color, color.rgb, beta);

    float a = max(color.a, beta);
#ifdef USE_ALPHA
    if (a < 0.7) {
      discard;
    }
#endif
  }
  rgb = materialColor.rgb;

  o_fragColor = rgb;
  o_fragMetal = material.metal;
  o_fragEmission = material.emission;
  o_fragNormal = encodeGNormal(normal, fs_in.viewPos);
}
