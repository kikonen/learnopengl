#pragma once

#include <string>

#include <freetype-gl/texture-font.h>

namespace text
{
    struct AtlasHandle;

    struct FontHandle {
        FontHandle(AtlasHandle* atlasHandle);
        FontHandle(FontHandle& o) = delete;
        FontHandle& operator=(FontHandle& o) = delete;
        FontHandle& operator=(FontHandle&& o) noexcept;
        FontHandle(FontHandle&& o) noexcept;
        ~FontHandle();

        bool valid() const;

        void create(
            const std::string& fullPath,
            float fontSize,
            int padding);

        AtlasHandle* m_atlasHandle;
        ftgl::texture_font_t* m_font{ nullptr };
    };
}
