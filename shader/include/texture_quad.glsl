//
// Based into idea from OGLDEV
// => no need to pass data for static quad via VAO
// => no need to change VAO binding
//
const vec3 VERTEX_POS[4] = vec3[4] (
  vec3(-1.0,  1.0, 0.0),
  vec3(-1.0, -1.0, 0.0),
  vec3( 1.0,  1.0, 0.0),
  vec3( 1.0, -1.0, 0.0)
);

const vec2 VERTEX_TEX_COORD[4] = vec2[4] (
  vec2(0.0, 1.0),
  vec2(0.0, 0.0),
  vec2(1.0, 1.0),
  vec2(1.0, 0.0)
);

const vec3 VERTEX_NORMAL = vec3(0, 0, 1);
const vec3 VERTEX_TANGENT = vec3(1, 0, 0);

const int VERTEX_INDECES[6] = int[6] (0, 1, 2, 2, 1, 3);
