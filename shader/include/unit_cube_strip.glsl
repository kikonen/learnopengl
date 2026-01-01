//
// Based into idea from OGLDEV
// => no need to pass data for static quad via VAO
// => no need to change VAO binding
//

// https://stackoverflow.com/questions/28375338/cube-using-single-gl-triangle-strip
const vec3 VERTEX_POS[8] = vec3[8] (
  vec3( 0.5,  0.5, -0.5),
  vec3(-0.5,  0.5, -0.5),
  vec3( 0.5, -0.5, -0.5),
  vec3(-0.5, -0.5, -0.5),
  vec3( 0.5,  0.5,  0.5),
  vec3(-0.5,  0.5,  0.5),
  vec3(-0.5, -0.5,  0.5),
  vec3( 0.5, -0.5,  0.5)
  );

const int VERTEX_INDECES[14] = int[14] (
  0, 1, 4, 5, 6, 1, 3, 0, 2, 4, 7, 6, 2, 3
  );
