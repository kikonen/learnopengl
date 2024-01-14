#pragma once

#include <memory>


#include "kigl/GLTextureHandle.h"

#include "text/size.h"

#include "text/FontHandle.h"
#include "text/AtlasHandle.h"

class Assets;
class RenderContext;

namespace text
{
    //
    // Prepare freetext-gl material
    //
    class FontAtlas
    {
    public:
        FontAtlas() {}
        FontAtlas(FontAtlas& o) = delete;
        FontAtlas& operator=(FontAtlas& o) = delete;
        FontAtlas& operator=(FontAtlas&& o) noexcept;
        FontAtlas(FontAtlas&& o) noexcept;
        ~FontAtlas();

        void prepare(
            const Assets& assets);

        void update();

        void bindTextures(kigl::GLState& state);
        void unbindTextures(kigl::GLState& state);

        FontHandle* getFont() {
            return m_fontHandle.get();
        }

    public:
        text::font_id m_id{ 0 };
        std::string m_name;

        std::string m_fontPath{ "fonts/Vera.ttf" };
        float m_fontSize{ 36.f };
        glm::uvec2 m_atlasSize{ 1024 };

    private:
        bool m_prepared{ false };

        std::unique_ptr<AtlasHandle> m_atlasHandle{ nullptr };
        size_t m_usedAtlasSize{ 0 };

        std::unique_ptr<FontHandle> m_fontHandle{ nullptr };

        kigl::GLTextureHandle m_texture;
    };
}
