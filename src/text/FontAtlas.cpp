#include "FontAtlas.h"

#include <algorithm>
#include <cmath>

#include <freetype-gl/texture-atlas.h>
#include <freetype-gl/texture-font.h>

#include "glm/ext.hpp"

#include "util/util.h"
#include "util/file.h"

#include "asset/Assets.h"

#include "shader/Shader.h"

#include "material/TextureSpec.h"
#include "material/RawTexture.h"
#include "material/ArrayTexture.h"
#include "material/TextureRegistry.h"

#include "AtlasHandle.h"
#include "FontHandle.h"

namespace {
    const uint32_t FONT_ATLAS_SIZE{ 1024 };

    const glm::vec3 BLACK{ 0.f };

    const std::string DEFAULT_FONT{ "fonts/Vera.ttf" };

    // Cap on the SDF raster resolution (freetype bake size). SDF magnifies
    // cleanly, so larger display text does not need a larger raster; capping
    // keeps the atlas bounded instead of growing quadratically with font size.
    // Sizes <= cap bake at their exact size (geometry scale stays 1.0), so
    // existing text is unchanged.
    constexpr float MAX_RASTER_SIZE{ 64.f };

    // Coarse atlas mips bleed neighbouring glyphs; cap the chain so bleed stays
    // within the per-glyph padding gutter while still covering minification.
    constexpr int MAX_MIP_LEVELS{ 4 };

    constexpr size_t ATLAS_DEPTH = 1;


    glm::uvec2 resolveAtlasSize(float rasterSize, float padding)
    {
        if (rasterSize < 8.f) rasterSize = 8.f;

        // 16 = glyphs per row == 16 * 16 = 256 glyphs
        constexpr float glyphsPerRow = 18.f;
        const float pz = rasterSize + padding;
        const float b = pz * glyphsPerRow + pz;

        return glm::vec2{ b, b };
    }
}

namespace text
{
    FontAtlas::FontAtlas()
        : m_fontPath{ DEFAULT_FONT },
        m_fontSize{ 32.f },
        m_padding{ 32 },
        m_atlasSize{ 0 }
    {}

    FontAtlas& FontAtlas::operator=(FontAtlas&& o) noexcept
    {
        m_id = o.m_id;
        m_prepared = o.m_prepared;
        m_name = o.m_name;
        m_fontPath = o.m_fontPath;
        m_fontSize = o.m_fontSize;
        m_padding = o.m_padding;
        m_rasterSize = o.m_rasterSize;
        m_atlasSize = o.m_atlasSize;
        m_texture = std::move(o.m_texture);
        m_atlasHandle = std::move(o.m_atlasHandle);
        m_fontHandle = std::move(o.m_fontHandle);

        return *this;
    }

    FontAtlas::FontAtlas(FontAtlas&& o) noexcept
        : m_id{ o.m_id },
        m_prepared{ o.m_prepared },
        m_name{ o.m_name },
        m_fontPath{ o.m_fontPath },
        m_fontSize{ o.m_fontSize},
        m_padding{ o.m_padding },
        m_rasterSize{ o.m_rasterSize },
        m_atlasSize{ o.m_atlasSize },
        m_texture{ std::move(o.m_texture) },
        m_atlasHandle{ std::move(o.m_atlasHandle) },
        m_fontHandle{ std::move(o.m_fontHandle) }
    {}

    FontAtlas::~FontAtlas()
    {
    }

    bool FontAtlas::operator==(const FontAtlas& o) const noexcept
    {
        return m_fontPath == o.m_fontPath &&
            m_fontSize == o.m_fontSize &&
            m_padding == o.m_padding &&
            m_atlasSize == o.m_atlasSize;
    }

    bool FontAtlas::valid() const
    {
        return m_texture && m_texture->getHandle() > 0;
    }

    GLuint64 FontAtlas::getTextureHandle() const noexcept
    {
        return m_texture ? m_texture->getHandle() : 0;
    }

    void FontAtlas::prepare()
    {
        if (m_prepared) return;
        m_prepared = true;

        const auto& assets = Assets::get();

        if (m_fontSize <= 0) return;

        // Raster resolution (SDF fidelity) is decoupled from display size and
        // capped. TextDraw scales glyph geometry back up via getGeometryScale().
        m_rasterSize = std::min(m_fontSize, MAX_RASTER_SIZE);

        m_padding = static_cast<int>(m_rasterSize);
        m_atlasSize = resolveAtlasSize(m_rasterSize, static_cast<float>(m_padding));

        // TODO KI need to derive this from shared place
        m_atlasSize = glm::uvec2{ FONT_ATLAS_SIZE };

        {
            m_atlasHandle = std::make_unique<AtlasHandle>();
            m_atlasHandle->create(m_atlasSize.x, m_atlasSize.y, ATLAS_DEPTH);
        }

        {
            m_fontHandle = std::make_unique<FontHandle>(m_atlasHandle.get());
            m_fontHandle->create(
                util::joinPath(assets.assetsDir, m_fontPath),
                m_rasterSize,
                m_padding);
        }

        if (!m_fontHandle->valid()) return;

        registerTexture();
    }

    void FontAtlas::registerTexture()
    {
        const GLsizei width = static_cast<GLsizei>(m_atlasHandle->m_atlas->width);
        const GLsizei height = static_cast<GLsizei>(m_atlasHandle->m_atlas->height);

        material::TextureSpec spec{
            .wrap = material::WrapMode::clamp_to_edge,
            .minFilter = material::TextureFilter::linear_mipmap_linear,
            .magFilter = material::TextureFilter::linear,
        };

        m_texture = util::Ref<RawTexture>::create(
            m_name,
            false,
            false,
            material::TextureType::map_font_atlas,
            spec,
            width,
            height
        );

        m_texture->setData(
            m_atlasHandle->m_atlas->data,
            width * height,
            GL_UNSIGNED_BYTE
        );

        TextureRegistry::get().registerTexture(m_texture);

        m_usedAtlasSize = m_atlasHandle->m_atlas->used;
    }

    void FontAtlas::update()
    {
        updateTexture();
    }

    void FontAtlas::updateTexture()
    {
        if (!valid()) return;

        size_t currentAtlasSize = m_atlasHandle->m_atlas->used;
        if (m_usedAtlasSize == currentAtlasSize) return;

        TextureRegistry::get().updateTexture(m_texture);

        m_usedAtlasSize = currentAtlasSize;
    }
}
