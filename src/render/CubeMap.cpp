#include "CubeMap.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "ki/GL.h"

#include "kigl/GLTextureHandle.h"
#include "kigl/GLState.h"

#include "asset/Image.h"
#include "asset/Program.h"

#include "render/TextureCube.h"
#include "render/FrameBuffer.h"
#include "render/RenderContext.h"

#include "registry/Registry.h"

namespace {
}

CubeMap::CubeMap(bool empty)
    : m_empty(empty)
{}

CubeMap::~CubeMap()
{
    // TODO KI delete texture
}

void CubeMap::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_hdri) {
        createHdri(assets, registry);
    } else if (m_irradiance) {
        createIrradiance(assets, registry);
    } else if (m_empty) {
        createEmpty();
    }
    else {
        createFaces();
    }
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
void CubeMap::createEmpty()
{
    GLuint textureID;

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureID);

    glTextureStorage2D(textureID, 1, m_internalFormat, m_size, m_size);

    glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    m_textureID = textureID;
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
    GLuint textureID;

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &textureID);

    int w = -1;
    int h = -1;

    std::vector<std::unique_ptr<Image>> images{ 6 };

    std::vector<GLenum> formats{};

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

    glTextureStorage2D(textureID, 1, m_internalFormat, w, h);

    // https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions
    for (unsigned int i = 0; i < 6; i++)
    {
        Image* image = images[i].get();
        if (!image || !image->m_data) continue;

        GLenum format = formats[i];
        glTextureSubImage3D(textureID, 0, 0, 0, i, image->m_width, image->m_height, 1, format, GL_UNSIGNED_BYTE, image->m_data);
    }

    glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    m_textureID = textureID;
}

void CubeMap::createHdri(
    const Assets& assets,
    Registry* registry)
{
    m_size = assets.skyboxSize;

    GLTextureHandle hdriTexture;
    {
        auto image = std::make_unique<Image>(m_faces[0], true, true);

        if (image->load()) {
            return;
        }

        const unsigned int width = image->m_width;
        const unsigned int height = image->m_height;

        {
            hdriTexture.create("hdri", GL_TEXTURE_2D);

            const GLenum internalFormat = GL_RGB16F;
            const GLenum pixelFormat = GL_FLOAT;
            const GLenum format = GL_RGB;

            glTextureStorage2D(hdriTexture, 1, internalFormat, width, height);
            glTextureSubImage2D(hdriTexture, 0, 0, 0, width, height, format, pixelFormat, image->m_data);

            glTextureParameteri(hdriTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(hdriTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(hdriTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(hdriTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

    {
        GLState state;

        auto program = registry->m_programRegistry->getProgram(SHADER_HDRI_CUBE_MAP);
        program->prepare(assets);

        program->bind(state);
        state.bindTexture(UNIT_HDR_TEXTURE, hdriTexture, false);

        renderCube(state, program);
    }

    m_textureID = m_cubeTexture;
}

void CubeMap::createIrradiance(
    const Assets& assets,
    Registry* registry)
{
    if (!m_envCubeMapRef) return;

    m_size = assets.irradianceMapSize;

    {
        GLState state;

        auto program = registry->m_programRegistry->getProgram(SHADER_IRRADIANCE_CUBE_MAP);
        program->prepare(assets);

        program->bind(state);
        state.bindTexture(UNIT_SKYBOX, *m_envCubeMapRef, false);

        renderCube(state, program);
    }

    m_textureID = m_cubeTexture;
}

void CubeMap::renderCube(
    GLState& state,
    Program* program)
{
    {
        m_cubeTexture.create("cube_map", GL_TEXTURE_CUBE_MAP);

        glTextureStorage2D(m_cubeTexture, 1, m_internalFormat, m_size, m_size);

        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_cubeTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    std::unique_ptr<FrameBuffer> captureFBO{ nullptr };
    {
        auto buffer = new FrameBuffer(
            "captureFBO",
            {
                m_size, m_size,
                {
                FrameBufferAttachment::getDrawBuffer(),
                FrameBufferAttachment::getRBODepth(),
            }
            });
        captureFBO.reset(buffer);
        captureFBO->prepare(true);
    }

    TextureCube cube;
    cube.prepare();

    {
        const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        const glm::mat4 captureViews[] =
        {
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
           glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };

        //program->setMat4("projection", captureProjection);

        glViewport(0, 0, m_size, m_size);
        glBindFramebuffer(GL_FRAMEBUFFER, *captureFBO);

        for (unsigned int i = 0; i < 6; ++i)
        {
            //program->setMat4("view", captureViews[i]);
            auto projected = captureProjection * captureViews[i];
            program->setMat4("projected", projected);

            glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                m_cubeTexture,
                0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cube.draw(state);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}
