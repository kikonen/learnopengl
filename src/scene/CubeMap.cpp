#include "CubeMap.h"

#include "ki/GL.h"

#include "asset/Image.h"

#include "scene/RenderContext.h"


CubeMap::CubeMap(bool empty)
    : m_empty(empty)
{}

CubeMap::~CubeMap()
{
    // TODO KI delete texture
}

void CubeMap::create()
{
    m_textureID = m_empty ? createEmpty() : createFaces();
}

void CubeMap::bindTexture(const RenderContext& ctx, int unitIndex)
{
    ctx.m_state.bindTexture(unitIndex, m_textureID, false);
}

//
// EMPTY cube map
//
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
GLuint CubeMap::createEmpty()
{
    unsigned int textureID;

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureID);

    glTextureStorage2D(textureID, 1, m_internalFormat, m_size, m_size);

    glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
unsigned int CubeMap::createFaces()
{
    unsigned int textureID;

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureID);

    int w = -1;
    int h = -1;

    std::vector<std::unique_ptr<Image>> images{ 6 };

    for (unsigned int i = 0; i < 6; i++)
    {
        images[i] = std::make_unique<Image>(m_faces[i]);
        Image* image = images[i].get();

        if (image->load(false)) continue;

        if (w == -1) {
            w = image->m_width;
            h = image->m_height;
        }
        else {
            if (w != image->m_width || h != image->m_height) {
                throw std::runtime_error{
                    fmt::format(
                        "cubemap texture size mismatch {}=({}, {}) vs. {}=({}, {})",
                        m_faces[0], w, h, m_faces[i], image->m_width, image->m_height) };
            }
        }
    }

    glTextureStorage2D(textureID, 1, m_internalFormat, w, h);

    // https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions
    for (unsigned int i = 0; i < 6; i++)
    {
        Image* image = images[i].get();
        if (!image || !image->m_data) continue;

        glTextureSubImage3D(textureID, 0, 0, 0, i, image->m_width, image->m_height, 1, m_format, GL_UNSIGNED_BYTE, image->m_data);
    }

    glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
