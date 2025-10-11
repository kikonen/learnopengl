#version 460 core

// layout (location = ATTR_POS) in vec3 a_pos;

// //uniform mat4 projection;
// //uniform mat4 view;
// uniform mat4 u_projected;

// out VS_OUT {
//   vec3 worldPos;
// } vs_out;

// ////////////////////////////////////////////////////////////
// //
// ////////////////////////////////////////////////////////////

// SET_FLOAT_PRECISION;

// void main()
// {
//   vs_out.worldPos = a_pos;
//   gl_Position =  u_projected * vec4(vs_out.worldPos, 1.0);
// }

uniform mat4 u_projected;

out VS_OUT {
  vec2 texCoord;
  vec3 worldPos;
  flat mat4 projected;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  // vs_out.texCoord = VERTEX_TEX_COORD[gl_VertexID];
  // gl_Position = vec4(VERTEX_POS[gl_VertexID], 1.0);

  // https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
  vec2 uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
  gl_Position = vec4(uv * 2.0f + -1.0f, 0.0f, 1.0f);

  vs_out.projected = u_projected;
  // vs_out.projected = mat4(0);
  vs_out.texCoord = uv;
}
