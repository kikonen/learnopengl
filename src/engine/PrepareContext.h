#pragma once

#include "asset/Assets.h"

class Registry;

//
// Context for doing prepare, without rendering
//
struct PrepareContext final {
public:
    PrepareContext(
        const Assets& assets,
        Registry* registry);

public:
    const Assets& m_assets;

    Registry* const m_registry;
};
