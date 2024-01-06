#version 460 core

#include struct_lights.glsl
#include struct_material.glsl
#include struct_shape.glsl

#include uniform_matrices.glsl
#include uniform_data.glsl
#include uniform_lights.glsl
#include uniform_materials.glsl
#include uniform_shapes.glsl

in VS_OUT {
  vec2 texCoord;

  flat uint materialIndex;
} fs_in;

layout(binding = UNIT_FONT_ATLAS) uniform sampler2D u_fontAtlas;

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

void main() {
  vec4  color = texture2D(u_fontAtlas, fs_in.texCoord.st);
  float dist  = color.r;
  float width = fwidth(dist);
  float alpha = smoothstep(glyph_center-width, glyph_center+width, dist);

  vec3 rgb = mix(glow_color, glyph_color, alpha);
  float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
  color = vec4(rgb, max(alpha,mu));
  float beta = smoothstep(outline_center-width, outline_center+width, dist);
  rgb = mix(outline_color, color.rgb, beta);

  float a = max(color.a, beta);
  if (a < 0.0001) {
    discard;
  }

  o_fragColor = vec4(rgb, a);
}
