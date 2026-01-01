#ifdef SCREEN_TRI_VERTEX_OUT
in VS_OUT {
  vec2 texCoord;
} fs_in;
#else
#include include/uniform_buffer_info.glsl
#endif
