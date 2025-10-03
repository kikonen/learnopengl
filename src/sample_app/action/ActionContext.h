#pragma once

#include "engine/InputContext.h"

namespace action
{
    struct ActionContext final : InputContext
    {
    public:
        ActionContext(
            Engine& engine,
            const Input& input);
    };
}
