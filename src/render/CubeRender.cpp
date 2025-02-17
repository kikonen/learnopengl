#include "CubeRender.h"

#include "kigl/GLTextureHandle.h"
#include "kigl/GLState.h"

#include "shader/Program.h"

#include "material/Image.h"

#include "render/TextureCube.h"
#include "render/FrameBuffer.h"

namespace render {
    void CubeRender::render(
        Program* program,
        int cubeTextureID,
        int size)
    {
        std::unique_ptr<FrameBuffer> captureFBO{ nullptr };
        {
            auto buffer = new FrameBuffer(
                "captureFBO",
                {
                    size, size,
                    {
                        FrameBufferAttachment::getDrawBuffer(),
                        FrameBufferAttachment::getDepthRbo(),
                    }
                });
            captureFBO.reset(buffer);
            captureFBO->prepare();
        }


        {
            const TextureCube& cube = TextureCube::get();

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

            glViewport(0, 0, size, size);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, *captureFBO);

            // HACK KI env cubemap fails to render from HDRI often (reason unknown)
            const glm::vec4 clearColor{ 0.8f, 0.8f, 0.9f, 0.f };
            const float clearDepth{ 1.f };

            for (unsigned int face = 0; face < 6; ++face)
            {
                //program->setMat4("view", captureViews[face]);
                auto projected = captureProjection * captureViews[face];
                program->setMat4("projected", projected);

                // NOTE KI side vs. face difference
                // https://stackoverflow.com/questions/55169053/opengl-render-to-cubemap-using-dsa-direct-state-access
                glNamedFramebufferTextureLayer(
                    *captureFBO,
                    GL_COLOR_ATTACHMENT0,
                    cubeTextureID,
                    0,
                    face);

                glClearNamedFramebufferfv(*captureFBO, GL_COLOR, 0, glm::value_ptr(clearColor));
                glClearNamedFramebufferfv(*captureFBO, GL_DEPTH, 0, &clearDepth);

                cube.draw();
            }
        }
    }
}
