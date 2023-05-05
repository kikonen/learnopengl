#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;

#include uniform_matrices.glsl

out VS_OUT {
  vec3 texCoord;
} vs_out;

out float gl_ClipDistance[2];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  vec4 pos = vec4(a_pos, 1.0);

  // https://www.rioki.org/2013/03/07/glsl-skybox.html
  // mat4 m = u_viewMatrix;
  // m[3][0] = 0.0;
  // m[3][2] = 0.0;
  // m[3][2] = 0.0;
  // vec4 v = inverse(m) * inverse(u_projectionMatrix) * pos;
  vec4 v = u_viewMatrixSkybox * pos;

  gl_Position = pos;
  vs_out.texCoord = v.xyz;

  gl_ClipDistance[0] = 1;
  gl_ClipDistance[1] = 1;
}
