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

    // Shared trivial selectors for sites that don't filter on that axis
    inline const std::function<bool(const render::DrawableInfo&)> ACCEPT_ALL_DRAWABLES =
        [](const render::DrawableInfo&) { return true; };
    inline const std::function<bool(const model::TypeFlags&)> ACCEPT_ALL_TYPES =
        [](const model::TypeFlags&) { return true; };

    struct DrawContext
    {
        // NOTE KI selectors are held BY VALUE. DrawContext is aggregate-initialized
        // (often from inline lambdas) and consumed on a later statement; reference
        // members would bind to temporaries that die at the end of the construction
        // full-expression. By value DrawContext owns them for its lifetime.

        // identity / selection filtering, keyed on DrawableInfo (entityIndex etc.)
        std::function<bool(const render::DrawableInfo&)> drawableSelector;

        // type-flag filtering
        std::function<bool(const model::TypeFlags&)> typeSelector;

        // rnder::KIND_NONE
        const uint8_t kindBits;

        // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
        const GLbitfield copyMask;
    };
}
