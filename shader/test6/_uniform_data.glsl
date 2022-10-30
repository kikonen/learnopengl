layout(std140) uniform Data {
  vec3 u_viewPos;
  float u_time;

  vec2 u_resolution;

  vec4 u_fogColor;
  float u_fogStart;
  float u_fogEnd;
};
