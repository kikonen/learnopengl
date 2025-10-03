#pragma once

#include "engine/BaseContext.h"

namespace gui
{
    struct FrameContext final : BaseContext
    {
        FrameContext(Engine& engine);
    };
}
