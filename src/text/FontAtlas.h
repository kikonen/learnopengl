#pragma once

#include <memory>

#include "util/Ref.h"

#include "kigl/GLTextureHandle.h"

#include "text/size.h"

#include "text/FontHandle.h"
#include "text/AtlasHandle.h"

class RawTexture;

namespace render
{
    class RenderContext;
}

namespace text
{
    //
    // Prepare freetext-gl material
    //
    class FontAtlas final : public util::RefCountedSimple
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

        GLuint64 getTextureHandle() const noexcept;

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

    private:
        void registerTexture();
        void updateTexture();

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

        std::unique_ptr<AtlasHandle> m_atlasHandle{ nullptr };
        size_t m_usedAtlasSize{ 0 };

        std::unique_ptr<FontHandle> m_fontHandle{ nullptr };

        util::Ref<RawTexture> m_texture;
    };
}
