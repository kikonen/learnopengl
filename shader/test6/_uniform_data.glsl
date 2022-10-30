layout(std140) uniform Data {
  vec3 iViewPos;
  float iTime;

  vec2 iResolution;

  vec4 iFogColor;
  float iFogStart;
  float iFogEnd;
};
