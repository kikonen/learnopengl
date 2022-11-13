// OpenGL Superbible, 7th Edition, page 552
// https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
// https://www.khronos.org/opengl/wiki/Bindless_Texture

// Enable bindless textures
// NOTE KI MUST BE inserted just after #version
//#extension GL_ARB_bindless_texture : require

layout (std140, binding = 6) uniform Textures
{
  uvec2 u_texture_handles[TEX_COUNT];
};
