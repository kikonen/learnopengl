#version 460 core

#include struct_material.glsl
#include struct_shape.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include ssbo_materials.glsl
#include ssbo_shapes.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  vec3 worldPos;
  vec3 normal;
  vec2 texCoord;
#ifdef USE_NORMAL_PATTERN
  vec3 vertexPos;
#endif

  vec2 atlasCoord;

  flat uint materialIndex;
  flat uint shapeIndex;

#ifdef USE_TBN
  vec3 tangent;
#endif
#ifdef USE_PARALLAX
  vec3 viewTangentPos;
  vec3 tangentPos;
#endif
} fs_in;

layout(binding = UNIT_CUBE_MAP) uniform samplerCube u_cubeMap;
layout(binding = UNIT_FONT_ATLAS) uniform sampler2D u_fontAtlas;

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

Material material;

#include fn_calculate_normal_pattern.glsl
#include fn_calculate_parallax_mapping.glsl
#include fn_gbuffer_encode.glsl

void main()
{
  material = u_materials[fs_in.materialIndex];

  #include var_tex_coord.glsl
  #include var_tex_material.glsl

  const vec3 viewDir = normalize(u_viewWorldPos - fs_in.worldPos);

#ifdef USE_ALPHA
#ifdef USE_BLEND_OIT
  if (material.diffuse.a < 0.95)
    discard;
#else
  if (material.diffuse.a < 0.05)
    discard;
#endif
#endif

  #include var_tex_material_normal.glsl

//  if (material.pattern == 1) {
//    normal = calculateNormalPattern(fs_in.vertexPos, normal);
//  }

  if (!gl_FrontFacing) {
    normal = -normal;
  }

#ifdef USE_CUBE_MAP
  #include var_calculate_cube_map_diffuse.glsl
#endif

  vec4 materialColor = material.diffuse;
  clamp_color(materialColor);

  // Font shaping
  vec3 rgb;
  {
    vec4  atlasValue = texture2D(u_fontAtlas, fs_in.atlasCoord.st);
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
    if (a < 0.7) {
      discard;
    }
  }
  rgb = materialColor.rgb;

  o_fragColor = vec4(rgb, 1.0);
  o_fragMetal = material.metal;
  o_fragEmission = material.emission.xyz;
  o_fragNormal = encodeGNormal(normal);
}
