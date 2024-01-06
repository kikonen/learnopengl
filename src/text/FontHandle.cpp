
#include "FontHandle.h"

#include "AtlasHandle.h"

namespace
{
    // https://stackoverflow.com/questions/50403342/how-do-i-properly-use-stdstring-on-utf-8-in-c
    // https://en.wikipedia.org/wiki/Specials_(Unicode_block)
    // https://stackoverflow.com/questions/47375068/storing-unicode-in-c-charcaters
    const char* CACHE =
        "? !\"#$%&'()*+,-./0123456789:;<=>?"
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
        "`abcdefghijklmnopqrstuvwxyz{|}~";

    // "? ÄÅÖäåö"
    const char* CACHE_MIN =
        "? \u00c4\u00c5\u00d6\u00e4\u00e5\u00f6";
}

namespace text
{
    FontHandle::FontHandle(AtlasHandle* atlasHandle)
        : m_atlasHandle(atlasHandle)
    {}

    FontHandle& FontHandle::operator=(FontHandle&& o) noexcept
    {
        m_atlasHandle = o.m_atlasHandle;
        m_font = o.m_font;
        o.m_font = nullptr;

        return *this;
    }

    FontHandle::FontHandle(FontHandle&& o) noexcept
        : m_atlasHandle{ o.m_atlasHandle },
        m_font{ o.m_font }
    {
        o.m_font = nullptr;
    }

    FontHandle::~FontHandle()
    {
        if (!m_font) return;
        // TODO KI triggers internal error on delete
        //ftgl::texture_font_delete(m_font);
    }

    void FontHandle::create(
        const std::string& fullPath,
        float fontSize)
    {
        m_font = ftgl::texture_font_new_from_file(
            m_atlasHandle->m_atlas,
            fontSize,
            fullPath.c_str());

        if (!m_font) return;

        m_font->rendermode = ftgl::RENDER_SIGNED_DISTANCE_FIELD;
        ftgl::texture_font_load_glyphs(m_font, CACHE);
    }
}
