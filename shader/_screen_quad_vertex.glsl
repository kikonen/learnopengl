#include texture_quad.glsl

out VS_OUT {
  vec2 texCoord;
} vs_out;

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////

SET_FLOAT_PRECISION;

void main()
{
  vs_out.texCoord = TEX_COORD[gl_VertexID];
  gl_Position = vec4(POS[gl_VertexID], 1.0);
}
