#version 460 core

layout (location = ATTR_POS) in vec4 a_pos;

//layout(location = UNIFORM_PROJECTION_MATRIX) uniform mat4 u_projectionMatrix;
//layout(location = UNIFORM_VIEW_MATRIX) uniform mat4 u_viewMatrix;

#include uniform_matrices.glsl

out vec3 texCoord;

out float gl_ClipDistance[2];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  texCoord = a_pos.xyz;
  vec4 pos = u_projectionMatrix * u_viewMatrixSkybox * a_pos;
  gl_Position = pos.xyww;

  gl_ClipDistance[0] = 1;
  gl_ClipDistance[1] = 1;
}
