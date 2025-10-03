#pragma once

#include "BaseContext.h"


//
// Context for doing prepare, without rendering
//
struct PrepareContext final : BaseContext {
public:
    PrepareContext(
        Engine& engine);
};
