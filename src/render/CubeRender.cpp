#include "CubeRender.h"

#include "kigl/GLTextureHandle.h"
#include "kigl/GLState.h"

#include "asset/Image.h"
#include "asset/Program.h"

#include "render/TextureCube.h"
#include "render/FrameBuffer.h"

void CubeRender::render(
    GLState& state,
    Program* program,
    GLTextureHandle cubeTexture)
{
    std::unique_ptr<FrameBuffer> captureFBO{ nullptr };
    {
        auto buffer = new FrameBuffer(
            "captureFBO",
            {
                cubeTexture.m_width, cubeTexture.m_height,
                {
                    FrameBufferAttachment::getDrawBuffer(),
                    FrameBufferAttachment::getRBODepth(),
                }
            });
        captureFBO.reset(buffer);
        captureFBO->prepare(true);
    }


    {
        TextureCube cube;
        cube.prepare();

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

        glViewport(0, 0, cubeTexture.m_width, cubeTexture.m_height);
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
                cubeTexture,
                0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cube.draw(state);
        }
    }
}
