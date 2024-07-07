#include "DebugContext.h"

namespace {
    static render::DebugContext s_instance;
}


namespace render
{
    const render::DebugContext& DebugContext::get() noexcept {
        return s_instance;
    }

    render::DebugContext& DebugContext::modify() noexcept {
        return s_instance;
    }
}
