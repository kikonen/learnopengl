#include "AtlasHandle.h"

namespace text
{
    AtlasHandle::~AtlasHandle()
    {
        if (!m_atlas) return;
        m_atlas->id = 0;
        ftgl::texture_atlas_delete(m_atlas);
    }

    void AtlasHandle::create(size_t w, size_t h, int depth)
    {
        m_atlas = ftgl::texture_atlas_new(w, h, depth);
    }
}
