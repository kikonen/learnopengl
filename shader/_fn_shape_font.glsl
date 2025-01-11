// Font shaping
// IN
// - material
//
void shapeFont(
  in uvec2 atlasHandle,
  in vec2 atlasCoord,
  in bool useBlend,
  out vec4 outColor)
{
  sampler2D atlas = sampler2D(atlasHandle);

  const float glyphCenter = 0.5;
  const float outlineCenter = 0.55;
  const float glowCenter = 1.05;
  const vec2 glowOffset = vec2(0, 0);
  // const vec2 glowOffset = vec2(0.003 * cos(u_time), 0.003 * sin(u_time));

  const vec3 fontColor = material.diffuse.rgb;
  const vec3 outlineColor  = vec3(0.0, 0.0, 1.0);
  const vec3 glowColor  = vec3(0.0, 1.0, 0.0);

  const float dist  = textureLod(atlas, atlasCoord, 0).r;
  const float width = fwidth(dist);

  const float alpha = smoothstep(
    glyphCenter - width,
    glyphCenter + width,
    dist);

  vec3 rgb;

  if (useBlend) {
    const float beta = smoothstep(
      outlineCenter - width,
      outlineCenter + width,
      dist);

    const float glowDist  = textureLod(atlas, atlasCoord + glowOffset, 0).r;
    const float glowWidth = fwidth(glowDist);
    const float glowAlpha = smoothstep(
      glowCenter - glowWidth,
      glowCenter + glowWidth,
      glowDist);

    rgb = mix(glowColor, fontColor, alpha);
    float mu = smoothstep(glyphCenter, glowCenter, sqrt(glowDist));
    outColor.rgb = rgb;
    outColor.a = max(alpha, mu);
  } else {
    rgb = fontColor;
    outColor.rgb = rgb;
    outColor.a = alpha;
  }

  // if (glowCenter > 0) {
  //   vec3 glowColor  = vec3(0.0, 1.0, 0.0);

  //   float glowDist  = 1.0 - textureLod(atlas, atlasCoord + glowOffset, 0).r;
  //   float glowWidth = fwidth(glowDist);

  //   float glowAlpha = 1.0 - smoothstep(glowCenter - glowWidth, glowCenter - glowWidth, glowDist);

  //   outColor.a = glyphAlpha + (1.0 - alpha) * alpha;
  //   outColor.rgb = mix(color, color, alpha / color.a);
  // } else {
  //   outColor.a = alpha;
  //   outColor.rgb = color;
  // }

  outColor.a *= material.diffuse.a;

  if (dist >= 0.9) {
    // outColor = vec4(1, 0, 0, 1);
  }
}
