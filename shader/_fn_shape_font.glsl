// Font shaping
// IN
// - material
//
void shapeFont(
  in uvec2 atlasHandle,
  in vec2 atlasCoord,
  in bool useBlend,
  out vec4 result)
{
#ifdef USE_OUTLINE
#ifdef USE_GLOW

#undef USE_OUTLINE
#undef USE_GLOW
#define USE_GLOW_OUTLINE

#endif
#endif

  sampler2D atlas = sampler2D(atlasHandle);

  const float glyphCenter = 0.5;
  const vec3 glyphColor = material.diffuse.rgb;

// #ifdef USE_OUTLINE
  const float outlineCenter = 0.55;
  // const vec3 outlineColor  = vec3(0.0, 0.0, 1.0);
  const vec3 outlineColor  = glyphColor * 0.5;
// #endif

// #ifdef USE_GLOW
  const float glowCenter = 1.05;
  const vec2 glowOffset = vec2(0, 0);
  // const vec2 glowOffset = vec2(0.003 * cos(u_time), 0.003 * sin(u_time));
  // const vec3 glowColor  = vec3(0.0, 1.0, 0.0);
  const vec3 glowColor = glyphColor * 1.3;
// #endif

  const float dist  = textureLod(atlas, atlasCoord, 0).r;
  const float width = fwidth(dist);

  const float alpha = smoothstep(
    glyphCenter - width,
    glyphCenter + width,
    dist);

  result = vec4(glyphColor, alpha);

#ifdef USE_OUTLINE
  if (useBlend) {
    float mu = smoothstep(outlineCenter - width, outlineCenter + width, dist);
    vec3 rgb = mix(outlineColor, glyphColor, mu);
    result = vec4(rgb, max(alpha, mu));
  }
#endif

#ifdef USE_GLOW
  if (useBlend) {
    vec3 rgb = mix(glowColor, glyphColor, alpha);
    float mu = smoothstep(glyphCenter, glowCenter, sqrt(dist));
    result = vec4(rgb, max(alpha, mu));
  }
#endif

#ifdef USE_GLOW_OUTLINE
  if (useBlend) {
    vec3 rgb = mix(glowColor, glyphColor, alpha);
    float mu = smoothstep(glyphCenter, glowCenter, sqrt(dist));
    vec4 color = vec4(rgb, max(alpha, mu));
    float beta = smoothstep(outlineCenter - width, outlineCenter + width, dist);
    rgb = mix(outlineColor, color.rgb, beta);
    result = vec4(rgb, max(color.a, beta));
  }
#endif

  // if (glowCenter > 0) {
  //   vec3 glowColor  = vec3(0.0, 1.0, 0.0);

  //   float glowDist  = 1.0 - textureLod(atlas, atlasCoord + glowOffset, 0).r;
  //   float glowWidth = fwidth(glowDist);

  //   float glowAlpha = 1.0 - smoothstep(glowCenter - glowWidth, glowCenter - glowWidth, glowDist);

  //   result.a = glyphAlpha + (1.0 - alpha) * alpha;
  //   result.rgb = mix(color, color, alpha / color.a);
  // } else {
  //   result.a = alpha;
  //   result.rgb = color;
  // }

  result.a *= material.diffuse.a;

  if (dist >= 0.9) {
    // result = vec4(1, 0, 0, 1);
  }
}
