layout(std140) uniform Data {
  vec3 viewPos;
  float time;

  vec4 fogColor;
  float fogStart;
  float fogEnd;
};
