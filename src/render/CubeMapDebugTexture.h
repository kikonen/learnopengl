#pragma once

#include <string>

#include "kigl/GLTextureHandle.h"

namespace render
{
    class CubeMapDebugTexture
    {
    public:
        CubeMapDebugTexture(
            const std::string& name);
        ~CubeMapDebugTexture();

        void prepare(int cubeSize);
        void render(
            const kigl::GLTextureHandle& cubeHandle,
            bool equirectangular);

    public:
        const std::string m_name;
        int m_cubeSize{ 0 };
        kigl::GLTextureHandle m_handle;
    };
}
