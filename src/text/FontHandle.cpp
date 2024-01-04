#include "FontHandle.h"

#include "AtlasHandle.h"

namespace
{
    const char* CACHE =
        " !\"#$%&'()*+,-./0123456789:;<=>?"
        "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
        "`abcdefghijklmnopqrstuvwxyz{|}~";
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
        ftgl::texture_font_delete(m_font);
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
