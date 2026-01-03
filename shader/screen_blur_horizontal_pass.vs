#version 460 core

// #include "include/screen_tri_vertex.glsl"

#include "include/uniform_buffer_info.glsl"

out VS_OUT {
  vec2 texCoords[9];
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

// https://www.youtube.com/watch?v=uZlwbWqQKpc
// https://vodacek.zvb.cz/archiv/251.html
void main()
{
  // https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
  vec2 uv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
  vec2 pos = uv * 2.0f + -1.0f;
  gl_Position = vec4(pos, 0.0f, 1.0f);

  const vec2 centerTexCoord = pos * 0.5 + 0.5;
  const float pixelSize = 1.0 / u_bufferResolution.x;
  for (int i = -4; i <= 4; i++) {
    vs_out.texCoords[i + 4] = centerTexCoord + vec2(pixelSize * i, 0.0);
  }
}
