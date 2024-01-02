#pragma once

#include <freetype-gl/texture-atlas.h>

namespace text
{
    struct AtlasHandle {
        AtlasHandle() = default;
        ~AtlasHandle();

        void create(size_t w, size_t h, int depth);

        ftgl::texture_atlas_t* m_atlas{ nullptr };
    };
}
