#include "DebugContext.h"

namespace {
    static debug::DebugContext s_instance;
}


namespace debug
{
    const debug::DebugContext& DebugContext::get() noexcept {
        return s_instance;
    }

    debug::DebugContext& DebugContext::modify() noexcept {
        return s_instance;
    }
}
