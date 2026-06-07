#pragma once

#include <functional>

#include "kigl/kigl.h"

#include <stdint.h>

namespace render
{
    struct DrawableInfo;

    // Shared trivial selector for sites that don't filter
    inline const std::function<bool(const render::DrawableInfo&)> ACCEPT_ALL_DRAWABLES =
        [](const render::DrawableInfo&) { return true; };

    struct DrawContext
    {
        // NOTE KI selector held BY VALUE. DrawContext is aggregate-initialized (often
        // from inline lambdas) and consumed on a later statement; a reference member
        // would bind to a temporary that dies at the end of the construction
        // full-expression. By value DrawContext owns it for its lifetime.

        // per-drawable filtering: drawable flags (d.m_flags) + identity/selection
        std::function<bool(const render::DrawableInfo&)> drawableSelector;

        // rnder::KIND_NONE
        const uint8_t kindBits;

        // GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
        const GLbitfield copyMask;
    };
}
