#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_TEX) in vec2 a_texCoord;

out vec2 texCoord;

layout(location = UNIFORM_VIEWPORT_TRANSFORM) uniform mat4 u_viewportTransform;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  texCoord = a_texCoord;
  gl_Position = u_viewportTransform * vec4(a_pos, 1.0);
}
