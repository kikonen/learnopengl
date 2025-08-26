#pragma once

#include "ki/RenderClock.h"

#include "engine/PrepareContext.h"

class Assets;
class Registry;

namespace debug {
    struct DebugContext;
}

//
// Context for doing updates, without rendering
//
struct UpdateContext final {
public:
    UpdateContext(
        const ki::RenderClock& clock,
        Registry* registry);

    operator PrepareContext() const
    {
        return toPrepareContext();
    }

    PrepareContext toPrepareContext() const;

public:
    const Assets& m_assets;
    const debug::DebugContext& m_dbg;
    const ki::RenderClock& m_clock;

    Registry* const m_registry;
};
