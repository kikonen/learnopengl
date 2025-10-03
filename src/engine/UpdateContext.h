#pragma once

#include "BaseContext.h"

//
// Context for doing updates, without rendering
//
struct UpdateContext final : BaseContext
{
public:
    UpdateContext(
        Engine& engine);
};
