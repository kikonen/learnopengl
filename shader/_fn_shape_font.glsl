// Font shaping
// IN
// - material
//
void shapeFont(
  in uvec2 atlasHandle,
  in vec2 atlasCoord,
  out vec4 color)
{
  sampler2D atlas = sampler2D(atlasHandle);

  //vec3 glyph_color    = vec3(1.0, 1.0, 1.0);
  const float glyph_center   = 0.5;

  // vec3 outline_color  = vec3(1.0, 0.0, 0.0);
  // const float outline_center = 1.2;

  vec3 glow_color     = vec3(1.0, 1.0, 1.0);
  const float glow_center    = 2.25;


  float dist  = 1.0 - textureLod(atlas, atlasCoord, 0).r;
  float width = 0.4;
  float edge = 0.3;

  float glyphAlpha = 1.0 - smoothstep(width, width + edge, dist);

  vec3 glyphColor = material.diffuse.rgb;

  // rgb = mix(glow_color, glyphColor, glyphAlpha);
  // float mu = smoothstep(glyph_center, glow_center, sqrt(dist));
  // rgb = glyphColor;

  // vec4 color = vec4(rgb, max(glyphAlpha, mu));

  float outlineWidth = 0.2;
  float outlineEdge = 0.4;

  if (outlineWidth > 0) {
    vec3 outlineColor  = vec3(0.0, 1.0, 0.0);
    vec2 outlineOffset = vec2(0, 0);

    float dist2  = 1.0 - textureLod(atlas, atlasCoord + outlineOffset, 0).r;

    float outlineAlpha = 1.0 - smoothstep(outlineWidth, outlineWidth + outlineEdge, dist2);

    color.a = glyphAlpha + (1.0 - glyphAlpha) * glyphAlpha;
    color.rgb = mix(outlineColor, glyphColor, glyphAlpha / color.a);
  } else {
    color.a = glyphAlpha;
    color.rgb = glyphColor;
  }
}
