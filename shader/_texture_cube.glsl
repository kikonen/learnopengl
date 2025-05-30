//
// Based into idea from OGLDEV
// => no need to pass data for static quad via VAO
// => no need to change VAO binding
//

const vec3 VERTEX_POS[24] = vec3[24] (
  // back
  vec3(-1.0,  1.0, -1.0),
  vec3( 1.0,  1.0, -1.0),
  vec3(-1.0, -1.0, -1.0),
  vec3( 1.0, -1.0, -1.0),
  // front
  vec3(-1.0,  1.0,  1.0),
  vec3(-1.0, -1.0,  1.0),
  vec3( 1.0,  1.0,  1.0),
  vec3( 1.0, -1.0,  1.0),
  // left
  vec3(-1.0,  1.0,  1.0),
  vec3(-1.0,  1.0, -1.0),
  vec3(-1.0, -1.0,  1.0),
  vec3(-1.0, -1.0, -1.0),
  // right
  vec3( 1.0, -1.0,  1.0),
  vec3( 1.0, -1.0, -1.0),
  vec3( 1.0,  1.0,  1.0),
  vec3( 1.0,  1.0, -1.0),
  // bottom
  vec3(-1.0, -1.0,  1.0),
  vec3(-1.0, -1.0, -1.0),
  vec3( 1.0, -1.0,  1.0),
  vec3( 1.0, -1.0, -1.0),
  // top
  vec3( 1.0,  1.0,  1.0),
  vec3( 1.0,  1.0, -1.0),
  vec3(-1.0,  1.0,  1.0),
  vec3(-1.0,  1.0, -1.0)
);

const vec3 VERTEX_NORMALS[24] = vec3[24] (
  // back
  vec3( 0.0,  0.0, -1.0),
  vec3( 0.0,  0.0, -1.0),
  vec3( 0.0,  0.0, -1.0),
  vec3( 0.0,  0.0, -1.0),
  // front
  vec3( 0.0,  0.0,  1.0),
  vec3( 0.0,  0.0,  1.0),
  vec3( 0.0,  0.0,  1.0),
  vec3( 0.0,  0.0,  1.0),
  // left
  vec3(-1.0,  0.0,  0.0),
  vec3(-1.0,  0.0,  0.0),
  vec3(-1.0,  0.0,  0.0),
  vec3(-1.0,  0.0,  0.0),
  // right
  vec3( 1.0,  0.0,  0.0),
  vec3( 1.0,  0.0,  0.0),
  vec3( 1.0,  0.0,  0.0),
  vec3( 1.0,  0.0,  0.0),
  // bottom
  vec3( 0.0, -1.0,  0.0),
  vec3( 0.0, -1.0,  0.0),
  vec3( 0.0, -1.0,  0.0),
  vec3( 0.0, -1.0,  0.0),
  // top
  vec3( 0.0,  1.0,  0.0),
  vec3( 0.0,  1.0,  0.0),
  vec3( 0.0,  1.0,  0.0),
  vec3( 0.0,  1.0,  0.0)
);

const vec3 VERTEX_TANGENTS[24] = vec3[24] (
  // back
  vec3(-1.0,  0.0,  0.0),
  vec3(-1.0,  0.0,  0.0),
  vec3(-1.0,  0.0,  0.0),
  vec3(-1.0,  0.0,  0.0),
  // front
  vec3( 1.0,  0.0,  0.0),
  vec3( 1.0,  0.0,  0.0),
  vec3( 1.0,  0.0,  0.0),
  vec3( 1.0,  0.0,  0.0),
  // left
  vec3( 0.0,  0.0,  1.0),
  vec3( 0.0,  0.0,  1.0),
  vec3( 0.0,  0.0,  1.0),
  vec3( 0.0,  0.0,  1.0),
  // right
  vec3( 0.0,  0.0, -1.0),
  vec3( 0.0,  0.0, -1.0),
  vec3( 0.0,  0.0, -1.0),
  vec3( 0.0,  0.0, -1.0),
  // bottom
  vec3(-1.0,  0.0,  0.0),
  vec3(-1.0,  0.0,  0.0),
  vec3(-1.0,  0.0,  0.0),
  vec3(-1.0,  0.0,  0.0),
  // top
  vec3( 1.0,  0.0,  0.0),
  vec3( 1.0,  0.0,  0.0),
  vec3( 1.0,  0.0,  0.0),
  vec3( 1.0,  0.0,  0.0)
);

const vec2 VERTEX_TEX_COORDS[24] = vec2[24] (
  // back
  vec2(0.0, 1.0),
  vec2(0.0, 0.0),
  vec2(1.0, 1.0),
  vec2(1.0, 0.0),
  // front
  vec2(0.0, 1.0),
  vec2(0.0, 0.0),
  vec2(1.0, 1.0),
  vec2(1.0, 0.0),
  // left
  vec2(0.0, 1.0),
  vec2(0.0, 0.0),
  vec2(1.0, 1.0),
  vec2(1.0, 0.0),
  // right
  vec2(0.0, 1.0),
  vec2(0.0, 0.0),
  vec2(1.0, 1.0),
  vec2(1.0, 0.0),
  // bottom
  vec2(0.0, 1.0),
  vec2(0.0, 0.0),
  vec2(1.0, 1.0),
  vec2(1.0, 0.0),
  // top
  vec2(0.0, 1.0),
  vec2(0.0, 0.0),
  vec2(1.0, 1.0),
  vec2(1.0, 0.0)
);

const int VERTEX_INDECES[36] = int[36] (
  // back
  0, 1, 2, 2, 1, 3,
  // front
  4, 5, 6, 6, 5, 7,
  // left
  8, 9, 10, 10, 9, 11,
  // right
  12, 13, 14, 14, 13, 15,
  // bottom
  16, 17, 18, 18, 17, 19,
  // top
  20, 21, 22, 22, 21, 23
  );
