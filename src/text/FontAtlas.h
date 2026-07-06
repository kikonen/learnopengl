#pragma once

#include <memory>


#include "kigl/GLTextureHandle.h"

#include "text/size.h"

#include "text/FontHandle.h"
#include "text/AtlasHandle.h"

namespace render
{
    class RenderContext;
}

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

        bool valid() const;

        void prepare();

        void update();

        FontHandle* getFont() const
        {
            return m_fontHandle.get();
        }

        GLuint64 getTextureHandle() const noexcept {
            return m_textureHandle;
        }

        int getPadding() const noexcept
        {
            return m_padding;
        }

        glm::uvec2 getAtlasSize() const noexcept
        {
            return m_atlasSize;
        }

        // Scale from atlas raster-pixel units (freetype metrics) to display
        // units. 1.0 while display size <= raster cap; > 1.0 once the raster
        // is capped for large text, so display size stays independent of the
        // SDF raster resolution baked into the atlas.
        float getGeometryScale() const noexcept
        {
            return m_rasterSize > 0.f ? m_fontSize / m_rasterSize : 1.f;
        }

    public:
        text::font_id m_id{ 0 };
        std::string m_name;

        std::string m_fontPath;
        // in points
        float m_fontSize;

    private:
        bool m_prepared{ false };

        int m_padding;
        // SDF raster resolution used to bake glyphs (fidelity knob), decoupled
        // from m_fontSize (display size). Capped so large text does not balloon
        // the atlas.
        float m_rasterSize{ 0.f };
        glm::uvec2 m_atlasSize;

        // number of mip levels allocated for the atlas texture (>1 == mipmapped)
        int m_mipLevels{ 1 };

        std::unique_ptr<AtlasHandle> m_atlasHandle{ nullptr };
        size_t m_usedAtlasSize{ 0 };

        std::unique_ptr<FontHandle> m_fontHandle{ nullptr };

        GLuint64 m_textureHandle{ 0 };
        kigl::GLTextureHandle m_texture;
    };
}
