#include "FontAtlas.h"

#include <freetype-gl/texture-atlas.h>
#include <freetype-gl/texture-font.h>

#include "asset/Shader.h"
#include "util/Util.h"

namespace {
    const char* CACHE =
        " !\"#$%&'()*+,-./0123456789:;<=>?"
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
        "`abcdefghijklmnopqrstuvwxyz{|}~";

    struct AtlastHandle {
        ~AtlastHandle()
        {
            if (!m_atlas) return;
            m_atlas->id = 0;
            ftgl::texture_atlas_delete(m_atlas);
        }

        void create(size_t w, size_t h, int depth)
        {
            m_atlas = ftgl::texture_atlas_new(w, h, depth);
        }

        ftgl::texture_atlas_t* m_atlas{ nullptr };
    };

    struct FontHandle {
        FontHandle(AtlastHandle& atlasHandle)
            : m_atlasHandle(atlasHandle)
        {}

        ~FontHandle()
        {
            if (!m_font) return;
            ftgl::texture_font_delete(m_font);
        }

        void create(
            const std::string& fullPath,
            float fontSize)
        {
            m_font = ftgl::texture_font_new_from_file(
                m_atlasHandle.m_atlas,
                fontSize,
                fullPath.c_str());

            if (!m_font) return;

            m_font->rendermode = ftgl::RENDER_SIGNED_DISTANCE_FIELD;
            ftgl::texture_font_load_glyphs(m_font, CACHE);
        }

        AtlastHandle& m_atlasHandle;
        ftgl::texture_font_t* m_font{ nullptr };
    };
}

void FontAtlas::prepareRT(
    const Assets& assets)
{
    AtlastHandle atlasHandle;
    const size_t depth = 1;
    atlasHandle.create(m_atlasSize.x, m_atlasSize.y, depth);

    {
        FontHandle fontHandle{ atlasHandle };
        fontHandle.create(
            util::joinPath(assets.assetsDir, m_fontName),
            m_fontSize);
    }

    const GLsizei w = static_cast<GLsizei>(atlasHandle.m_atlas->width);
    const GLsizei h = static_cast<GLsizei>(atlasHandle.m_atlas->height);

    m_texture.create("FONT_ATLAS", GL_TEXTURE_2D, w, h);
    atlasHandle.m_atlas->id = m_texture.m_textureID;
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
    glTextureSubImage2D(texId, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, atlasHandle.m_atlas->data);
    glGenerateTextureMipmap(texId);
}

void FontAtlas::bindTextures(GLState& state)
{
    m_texture.bindTexture(state, UNIT_FONT_ATLAS);
}

void FontAtlas::unbindTextures(GLState& state)
{
    m_texture.unbindTexture(state, UNIT_FONT_ATLAS);
}
