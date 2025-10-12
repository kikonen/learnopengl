#pragma once

#include <memory>

#include "editor/tool/ToolState.h"

namespace render
{
    class CubeMapDebugTexture;
}

namespace editor {
    struct ViewportToolState : public ToolState {

        bool m_equirectangular{ false };

        std::unique_ptr<render::CubeMapDebugTexture> m_environmentTexture;
        std::unique_ptr<render::CubeMapDebugTexture> m_irradianceTexture;
        std::unique_ptr<render::CubeMapDebugTexture> m_prefilterTexture;
        std::unique_ptr<render::CubeMapDebugTexture> m_skyboxTexture;
    };
}
