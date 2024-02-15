#pragma once

#include "ki/RenderClock.h"

#include "engine/PrepareContext.h"

class Assets;
class Registry;

//
// Context for doing updates, without rendering
//
struct UpdateContext final {
public:
    UpdateContext(
        const ki::RenderClock& clock,
        Registry* registry);

    PrepareContext toPrepareContext() const;

public:
    const Assets& m_assets;
    const ki::RenderClock& m_clock;

    Registry* const m_registry;
};
