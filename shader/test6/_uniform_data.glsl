layout(std140, binding = UBO_DATA) uniform Data {
  vec3 u_viewPos;
  float u_time;

  vec2 u_resolution;

  vec4 u_fogColor;
  float u_fogStart;
  float u_fogEnd;
};
