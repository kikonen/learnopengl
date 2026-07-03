#pragma once

#include "kigl/GLFrameBufferHandle.h"

class Program;

namespace render {
    class CubeRender {
    public:
        void render(
            Program* program,
            int cubeTextureID,
            int size);

    private:
        // NOTE KI persistent capture FBO: created lazily once, reused across render()
        // calls. Callers that keep a CubeRender member (e.g. repeated convolves) avoid
        // recreating the framebuffer every call.
        kigl::GLFrameBufferHandle m_fbo;
    };
}
