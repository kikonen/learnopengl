layout (location = ATTR_POS) in vec3 a_pos;
layout (location = ATTR_TEX) in vec2 a_texCoord;

out VS_OUT {
  vec2 texCoord;
} vs_out;

void main()
{
  vs_out.texCoord = a_texCoord;
  gl_Position = vec4(a_pos, 1.0);
}
