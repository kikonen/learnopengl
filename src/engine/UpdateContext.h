#pragma once

#include "ki/RenderClock.h"

#include "asset/Assets.h"

#include "engine/PrepareContext.h"

class Registry;

//
// Context for doing updates, without rendering
//
struct UpdateContext final {
public:
    UpdateContext(
        const ki::RenderClock& clock,
        const Assets& assets,
        Registry* registry);

    PrepareContext toPrepareContext() const;

public:
    const ki::RenderClock& m_clock;

    const Assets& m_assets;

    Registry* const m_registry;
};
