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

#include "AtlasHandle.h"
#include "FontHandle.h"

namespace {
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
        m_mipLevels = o.m_mipLevels;
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
        m_mipLevels{ o.m_mipLevels },
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
        return m_fontHandle && m_fontHandle->valid();
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

        constexpr size_t depth = 1;
        {
            m_atlasHandle = std::make_unique<AtlasHandle>();
            m_atlasHandle->create(m_atlasSize.x, m_atlasSize.y, depth);
        }

        {
            m_fontHandle = std::make_unique<FontHandle>(m_atlasHandle.get());
            m_fontHandle->create(
                util::joinPath(assets.assetsDir, m_fontPath),
                m_rasterSize,
                m_padding);
        }

        if (!m_fontHandle->valid()) return;


        if (true)
        {
            const GLsizei w = static_cast<GLsizei>(m_atlasHandle->m_atlas->width);
            const GLsizei h = static_cast<GLsizei>(m_atlasHandle->m_atlas->height);

            m_texture.create(fmt::format("{}_font_atlas", m_name), GL_TEXTURE_2D, w, h);
            m_atlasHandle->m_atlas->id = m_texture.m_textureID;
            const auto texId = m_texture.m_textureID;

            GLenum internalFormat;
            GLenum format;

            switch (depth) {
            case 1:
                internalFormat = GL_R8;
                format = GL_RED;
                break;
            case 3:
                internalFormat = GL_RGB8;
                format = GL_RGB;
                break;
            }

            // Mipmaps let minified / distant text sample coarser SDF levels
            // instead of shimmering. Distance fields downsample cleanly under
            // averaging (unlike alpha coverage), so a plain mip chain works;
            // capped to keep coarse-level bleed within the glyph padding gutter.
            m_mipLevels = std::clamp(
                1 + static_cast<int>(std::floor(std::log2(static_cast<float>(std::max(w, h))))),
                1,
                MAX_MIP_LEVELS);

            glTextureParameteri(texId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(texId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(texId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(texId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTextureParameterfv(texId, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(BLACK));

            glTextureStorage2D(texId, m_mipLevels, internalFormat, w, h);
            glTextureSubImage2D(texId, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, m_atlasHandle->m_atlas->data);
            if (m_mipLevels > 1) {
                glGenerateTextureMipmap(texId);
            }

            m_textureHandle = glGetTextureHandleARB(m_texture);
            glMakeTextureHandleResidentARB(m_textureHandle);

            m_usedAtlasSize = m_atlasHandle->m_atlas->used;
        }
    }

    void FontAtlas::update()
    {
        if (!valid()) return;

        size_t currentAtlasSize = m_atlasHandle->m_atlas->used;
        if (m_usedAtlasSize == currentAtlasSize) return;

        const GLsizei w = static_cast<GLsizei>(m_atlasHandle->m_atlas->width);
        const GLsizei h = static_cast<GLsizei>(m_atlasHandle->m_atlas->height);

        glTextureSubImage2D(
            m_texture.m_textureID,
            0,
            0, 0, w, h,
            GL_RED,
            GL_UNSIGNED_BYTE,
            m_atlasHandle->m_atlas->data);

        // newly rasterized glyphs changed level 0 -> refresh the mip chain
        if (m_mipLevels > 1) {
            glGenerateTextureMipmap(m_texture.m_textureID);
        }

        m_usedAtlasSize = currentAtlasSize;
    }
}
