#pragma once

#include <string>

#include "kigl/GLTextureHandle.h"

namespace render
{
    class CubeMapDebugTexture
    {
    public:
        CubeMapDebugTexture(
            const std::string& name,
            int cubeSize);
        ~CubeMapDebugTexture();

        void prepare();
        void render(
            const kigl::GLTextureHandle& cubeHandle,
            bool equirectangular);

    public:
        const std::string m_name;
        const int m_cubeSize;
        kigl::GLTextureHandle m_handle;
    };
}
