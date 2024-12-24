#version 460 core

#include struct_material.glsl
#include struct_resolved_material.glsl

#include ssbo_materials.glsl

#ifndef USE_ALPHA
// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
// https://www.gamedev.net/forums/topic/700517-performance-question-alpha-texture-vs-frag-shader-discard/5397906/
layout(early_fragment_tests) in;
#endif

in VS_OUT {
  vec2 atlasCoord;
  flat uvec2 atlasHandle;
  flat uint highlightIndex;
} fs_in;

layout (location = 0) out vec4 o_fragColor;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

vec3 glyph_color    = vec3(1.0, 1.0, 1.0);
const float glyph_center   = 0.50;

vec3 outline_color  = vec3(0.0, 0.0, 0.0);
const float outline_center = 0.55;

vec3 glow_color     = vec3(1.0, 1.0, 1.0);
const float glow_center    = 1.25;


void main()
{
  sampler2D atlas = sampler2D(fs_in.atlasHandle);
  vec4 color = texture2D(atlas, fs_in.atlasCoord.st);

  float dist  = color.r;
  float width = fwidth(dist);
  float alpha = smoothstep(glyph_center-width, glyph_center+width, dist);

  vec3 rgb = mix(glow_color, glyph_color, alpha);
  float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
  color = vec4(rgb, max(alpha,mu));
  float beta = smoothstep(outline_center-width, outline_center+width, dist);

  float a = max(color.a, beta);
  if (a < 0.6) {
    discard;
  }

  const uint materialIndex = fs_in.highlightIndex;
  o_fragColor = u_materials[materialIndex].diffuse;
}
