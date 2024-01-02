#pragma once

#include <memory>

#include "asset/Assets.h"

#include "kigl/GLTextureHandle.h"

#include "text/FontHandle.h"
#include "text/AtlasHandle.h"

class RenderContext;

namespace text
{
    //
    // Prepare freetext-gl material
    //
    class FontAtlas
    {
    public:
        FontAtlas()
        {}

        void prepareRT(
            const Assets& assets);

        void bindTextures(GLState& state);
        void unbindTextures(GLState& state);

    public:
        std::string m_fontName{ "fonts/Vera.ttf" };
        float m_fontSize{ 10.f };
        glm::uvec2 m_atlasSize{ 512, 512 };

    private:
        GLTextureHandle m_texture;

        std::unique_ptr<AtlasHandle> m_atlasHandle{ nullptr };
        std::unique_ptr<FontHandle> m_fontHandle{ nullptr };
    };
}
