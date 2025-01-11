#pragma once

#include <memory>


#include "kigl/GLTextureHandle.h"

#include "text/size.h"

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
        FontAtlas();
        FontAtlas(FontAtlas& o) = delete;
        FontAtlas& operator=(FontAtlas& o) = delete;
        FontAtlas& operator=(FontAtlas&& o) noexcept;
        FontAtlas(FontAtlas&& o) noexcept;
        ~FontAtlas();

        bool operator==(const FontAtlas& o) const noexcept;

        void prepare();

        void update();

        FontHandle* getFont() {
            return m_fontHandle.get();
        }

        GLuint64 getTextureHandle() const noexcept {
            return m_textureHandle;
        }

    public:
        text::font_id m_id{ 0 };
        std::string m_name;

        std::string m_fontPath;
        float m_fontSize;
        int m_padding;
        glm::uvec2 m_atlasSize;

    private:
        bool m_prepared{ false };

        std::unique_ptr<AtlasHandle> m_atlasHandle{ nullptr };
        size_t m_usedAtlasSize{ 0 };

        std::unique_ptr<FontHandle> m_fontHandle{ nullptr };

        GLuint64 m_textureHandle{ 0 };
        kigl::GLTextureHandle m_texture;
    };
}
