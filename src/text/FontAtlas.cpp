#include "FontAtlas.h"

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
    glm::vec3 BLACK{ 0.f };

    glm::uvec2 resolveAtlasSize(float fontSize, float padding)
    {
        if (fontSize < 8.f) fontSize = 8.f;

        // 16 = glyphs per row == 16 * 16 = 256 glyphs
        constexpr float glyphsPerRow = 18.f;
        const float pz = fontSize + padding;
        const float b = pz * glyphsPerRow + pz;

        return glm::vec2{ b, b };
    }
}

namespace text
{
    FontAtlas::FontAtlas()
        : m_fontPath{ "fonts/Vera.ttf" },
        m_fontSize{ 32.f },
        m_padding{ 32 },
        m_atlasSize{ 0 }
    {}

    FontAtlas& FontAtlas::operator=(FontAtlas&& o) noexcept
    {
        m_id = o.m_id;
        m_name = o.m_name;
        m_fontPath = o.m_fontPath;
        m_fontSize = o.m_fontSize;
        m_padding = o.m_padding;
        m_atlasSize = o.m_atlasSize;
        m_texture = std::move(o.m_texture);
        m_atlasHandle = std::move(o.m_atlasHandle);
        m_fontHandle = std::move(o.m_fontHandle);

        return *this;
    }

    FontAtlas::FontAtlas(FontAtlas&& o) noexcept
        : m_id{ o.m_id },
        m_name{ o.m_name },
        m_fontPath{ o.m_fontPath },
        m_fontSize{ o.m_fontSize},
        m_padding{ o.m_padding },
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

    void FontAtlas::prepare()
    {
        if (m_prepared) return;
        m_prepared = true;

        const auto& assets = Assets::get();

        if (m_fontSize <= 0) return;

        m_padding = static_cast<int>(m_fontSize);
        m_atlasSize = resolveAtlasSize(m_fontSize, m_padding);

        constexpr size_t depth = 1;
        {
            m_atlasHandle = std::make_unique<AtlasHandle>();
            m_atlasHandle->create(m_atlasSize.x, m_atlasSize.y, depth);
        }

        {
            m_fontHandle = std::make_unique<FontHandle>(m_atlasHandle.get());
            m_fontHandle->create(
                util::joinPath(assets.assetsDir, m_fontPath),
                m_fontSize,
                m_padding);
        }


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

            glTextureParameteri(texId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(texId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(texId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(texId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameterfv(texId, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(BLACK));

            //const int mipMapLevels = static_cast<int>(log2(std::max(w, h)));
            glTextureStorage2D(texId, 1, internalFormat, w, h);
            glTextureSubImage2D(texId, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, m_atlasHandle->m_atlas->data);
            //glGenerateTextureMipmap(texId);

            m_textureHandle = glGetTextureHandleARB(m_texture);
            glMakeTextureHandleResidentARB(m_textureHandle);

            m_usedAtlasSize = m_atlasHandle->m_atlas->used;
        }
    }

    void FontAtlas::update()
    {
        size_t currentAtlasSize = m_atlasHandle->m_atlas->used;
        if (m_usedAtlasSize == currentAtlasSize) return;

        const GLsizei w = static_cast<GLsizei>(m_atlasHandle->m_atlas->width);
        const GLsizei h = static_cast<GLsizei>(m_atlasHandle->m_atlas->height);

        glTextureSubImage2D(
            m_texture.m_textureID,
            0,
            0, 0, w, h,
            GL_RED,
            GL_UNSIGNED_BYTE, m_atlasHandle->m_atlas->data);

        m_usedAtlasSize = currentAtlasSize;
    }
}
