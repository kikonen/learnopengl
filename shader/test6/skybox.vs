#version 450 core
layout (location = 0) in vec3 aPos;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;

out vec3 texCoords;

out float gl_ClipDistance[2];

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

void main() {
  texCoords = aPos;
  vec4 pos = u_projectionMatrix * u_viewMatrix * vec4(aPos, 1.0);
  gl_Position = pos.xyww;

  gl_ClipDistance[0] = 1;
  gl_ClipDistance[1] = 1;
}
