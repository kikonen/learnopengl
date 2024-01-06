#version 460 core

in VS_OUT {
  vec2 texCoord;
} fs_in;

layout(binding = UNIT_FONT_ATLAS) uniform sampler2D u_fontAtlas;

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
  const vec2 texCoord = fs_in.texCoord;

  vec4  color = texture2D(u_fontAtlas, fs_in.texCoord.st);
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
}
