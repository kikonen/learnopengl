#version 460 core

#include texture_quad.glsl

out VS_OUT {
  vec2 texCoord;
} vs_out;

layout(location = UNIFORM_VIEWPORT_TRANSFORM) uniform mat4 u_viewportTransform;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  vs_out.texCoord = VERTEX_TEX_COORD[gl_VertexID];
  gl_Position = u_viewportTransform * vec4(VERTEX_POS[gl_VertexID], 1.0);
}
