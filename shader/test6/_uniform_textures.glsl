// OpenGL Superbible, 7th Edition, page 552
// https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures

// Enable bindless textures
//#extension GL_ARB_bindless_texture : require

layout (binding = 6, std140) uniform Textures
{
  uvec2 u_texture_handles[TEX_COUNT];
};
