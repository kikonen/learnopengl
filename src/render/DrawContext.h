#pragma once

#include <functional>

#include "kigl/kigl.h"

#include <stdint.h>

namespace model
{
    struct TypeFlags;
}


namespace render
{
    struct DrawableInfo;

    // Shared trivial selectors (static lifetime => safe to bind to DrawContext's
    // const& members at sites that don't filter on that axis)
    inline const std::function<bool(const render::DrawableInfo&)> ACCEPT_ALL_DRAWABLES =
        [](const render::DrawableInfo&) { return true; };
    inline const std::function<bool(const model::TypeFlags&)> ACCEPT_ALL_TYPES =
        [](const model::TypeFlags&) { return true; };

    struct DrawContext
    {
        // identity / selection filtering, keyed on DrawableInfo (entityIndex etc.)
        const std::function<bool(const render::DrawableInfo&)>& drawableSelector;

        // type-flag filtering
        const std::function<bool(const model::TypeFlags&)>& typeSelector;

        // rnder::KIND_NONE
        const uint8_t kindBits;

        // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
        const GLbitfield copyMask;
    };
}
