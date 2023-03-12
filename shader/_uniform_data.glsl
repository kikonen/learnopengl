layout(std140, binding = UBO_DATA) uniform Data {
  vec3 u_viewWorldPos;
  vec3 u_viewFront;
  vec3 u_viewUp;
  vec3 u_viewRight;

  float u_time;

  vec2 u_resolution;

  vec4 u_fogColor;
  float u_fogStart;
  float u_fogEnd;
  float u_fogRatio;

  bool u_cubeMapExist;

  //uint pad2;
};
