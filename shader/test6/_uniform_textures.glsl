// Enable bindless textures
#extension GL_ARB_bindless_texture : require

layout (binding = 6, std140) uniform Textures
{
  sampler2D u_textures[TEX_COUNT];
};
