#include "CubeRender.h"

#include <fmt/format.h>

#include "util/glm_format.h"

#include "kigl/GLTextureHandle.h"
#include "kigl/GLFrameBufferHandle.h"
#include "kigl/GLState.h"

#include "shader/Program.h"

#include "material/Image.h"

#include "render/TextureCube.h"

namespace render {
    void CubeRender::render(
        Program* program,
        int cubeTextureID,
        int size)
    {
        auto& state = kigl::GLState::get();

        kigl::GLFrameBufferHandle fbo;
        {
            fbo.create("cube_map_capture_fbo");

            glViewport(0, 0, size, size);
            state.bindFrameBuffer(fbo, true);
        }

        {
            // NTOE KI cube drawn from inside-out
            state.frontFace(GL_CW);

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

            const glm::vec4 clearColor{ 0.f, 0.f, 0.f, 0.f };

            program->bind();

            for (unsigned int face = 0; face < 6; ++face)
            {
                auto projected = captureProjection * captureViews[face];
                program->setMat4("u_projected", projected);

                //KI_INFO_OUT(fmt::format("projection: {}", captureProjection));
                //KI_INFO_OUT(fmt::format("face: {}", captureViews[face]));
                //KI_INFO_OUT(fmt::format("u_projected: {}", projected));

                // NOTE KI side vs. face difference
                // https://stackoverflow.com/questions/55169053/opengl-render-to-cubemap-using-dsa-direct-state-access
                glNamedFramebufferTextureLayer(
                    fbo,
                    GL_COLOR_ATTACHMENT0,
                    cubeTextureID,
                    0,
                    face);

                glClearNamedFramebufferfv(fbo, GL_COLOR, 0, glm::value_ptr(clearColor));

                cube.draw();
            }

            state.frontFace(GL_CCW);
        }
    }
}
