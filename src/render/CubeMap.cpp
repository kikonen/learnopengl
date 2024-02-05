#include "CubeMap.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"
#include "kigl/GLState.h"

#include "asset/Image.h"
#include "asset/Program.h"

#include "render/TextureCube.h"
#include "render/FrameBuffer.h"
#include "render/RenderContext.h"

#include "registry/Registry.h"

#include "CubeRender.h"


namespace render {
    CubeMap::CubeMap(
        std::string_view name,
        bool empty)
        : m_name(name),
        m_empty(empty)
    {}

    CubeMap::~CubeMap()
    {
        // TODO KI delete texture
    }

    void CubeMap::prepareRT(
        const PrepareContext& ctx)
    {
        if (m_empty) {
            createEmpty();
        }
        else {
            createFaces();
        }
    }

    void CubeMap::bindTexture(const RenderContext& ctx, int unitIndex)
    {
        auto& state = ctx.m_state;
        state.bindTexture(unitIndex, m_cubeTexture, false);
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
    void CubeMap::createEmpty()
    {
        m_cubeTexture.create(fmt::format("{}_empty_cube_map", m_name), GL_TEXTURE_CUBE_MAP, m_size, m_size);

        glTextureStorage2D(m_cubeTexture, 1, m_internalFormat, m_size, m_size);

        glTextureParameteri(m_cubeTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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
    void CubeMap::createFaces()
    {
        std::vector<std::unique_ptr<Image>> images{ 6 };
        std::vector<GLenum> formats{};

        {
            int w = -1;
            int h = -1;

            for (unsigned int i = 0; i < 6; i++)
            {
                images[i] = std::make_unique<Image>(m_faces[i], false);
                Image* image = images[i].get();

                if (image->load()) {
                    formats.emplace_back(GL_RGB);
                    continue;
                }

                {
                    GLenum format;
                    if (image->m_channels == 4) {
                        format = GL_RGBA;
                    }
                    else {
                        format = GL_RGB;
                    }
                    formats.emplace_back(format);
                }

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

            if (w != h) {
                throw std::runtime_error{
                    fmt::format(
                        "cubemap width != height: {}, {} != {}",
                        m_faces[0], w, h) };
            }

            m_size = w;
        }

        {
            m_cubeTexture.create(fmt::format("{}_face_cube_map", m_name), GL_TEXTURE_CUBE_MAP, m_size, m_size);

            glTextureStorage2D(m_cubeTexture, 1, m_internalFormat, m_size, m_size);

            // https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions
            for (unsigned int i = 0; i < 6; i++)
            {
                Image* image = images[i].get();
                if (!image || !image->m_data) continue;

                GLenum format = formats[i];
                glTextureSubImage3D(m_cubeTexture, 0, 0, 0, i, m_size, m_size, 1, format, GL_UNSIGNED_BYTE, image->m_data);
            }

            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glTextureParameteri(m_cubeTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }
}
