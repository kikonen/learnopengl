#include "FontAtlas.h"

#include <freetype-gl/texture-atlas.h>
#include <freetype-gl/texture-font.h>

#include "asset/Shader.h"
#include "util/Util.h"

#include "AtlasHandle.h"
#include "FontHandle.h"

namespace {
}

namespace text
{
    void FontAtlas::prepareRT(
        const Assets& assets)
    {
        const size_t depth = 1;
        {
            m_atlasHandle = std::make_unique<AtlasHandle>();
            m_atlasHandle->create(m_atlasSize.x, m_atlasSize.y, depth);
        }

        {
            m_fontHandle = std::make_unique<FontHandle>(m_atlasHandle.get());
            m_fontHandle->create(
                util::joinPath(assets.assetsDir, m_fontName),
                m_fontSize);
        }


        {
            const GLsizei w = static_cast<GLsizei>(m_atlasHandle->m_atlas->width);
            const GLsizei h = static_cast<GLsizei>(m_atlasHandle->m_atlas->height);

            m_texture.create("FONT_ATLAS", GL_TEXTURE_2D, w, h);
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

            const int mipMapLevels = static_cast<int>(log2(std::max(w, h)));
            glTextureStorage2D(texId, mipMapLevels, internalFormat, w, h);
            glTextureSubImage2D(texId, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, m_atlasHandle->m_atlas->data);
            glGenerateTextureMipmap(texId);
        }
    }

    void FontAtlas::bindTextures(GLState& state)
    {
        m_texture.bindTexture(state, UNIT_FONT_ATLAS);
    }

    void FontAtlas::unbindTextures(GLState& state)
    {
        m_texture.unbindTexture(state, UNIT_FONT_ATLAS);
    }
}
