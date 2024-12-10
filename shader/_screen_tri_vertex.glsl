// #include texture_quad.glsl

out VS_OUT {
  vec2 texCoord;
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

  vs_out.texCoord = uv;
}
