#version 460 core

layout (location = ATTR_POS) in vec3 a_pos;

uniform mat4 projection;
uniform mat4 view;

out VS_OUT {
  vec3 worldPos;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  vs_out.worldPos = a_pos;
  gl_Position =  projection * view * vec4(vs_out.worldPos, 1.0);
}
