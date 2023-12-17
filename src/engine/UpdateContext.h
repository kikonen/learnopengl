#pragma once

#include "ki/RenderClock.h"

#include "asset/Assets.h"

class Registry;

//
// Context for doing updates, without rendering
//
class UpdateContext {
public:
    UpdateContext(
        const ki::RenderClock& clock,
        const Assets& assets,
        Registry* registry);

public:
    const ki::RenderClock& m_clock;

    const Assets& m_assets;

    Registry* const m_registry;
};
