#pragma once

#include <string>

#include <freetype-gl/texture-font.h>

namespace text
{
    struct AtlasHandle;

    struct FontHandle {
        FontHandle(AtlasHandle* atlasHandle);
        ~FontHandle();

        void create(
            const std::string& fullPath,
            float fontSize);

        AtlasHandle* m_atlasHandle;
        ftgl::texture_font_t* m_font{ nullptr };
    };
}
