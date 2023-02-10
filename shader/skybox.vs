#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;

#include uniform_matrices.glsl

out vec3 texCoord;

out float gl_ClipDistance[2];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  texCoord = a_pos.xyz;
  vec4 projectedPos = u_projectionMatrix * u_viewMatrixSkybox * vec4(a_pos, 1.0);
  gl_Position = projectedPos.xyww;

  gl_ClipDistance[0] = 1;
  gl_ClipDistance[1] = 1;
}
