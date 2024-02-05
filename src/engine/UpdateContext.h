#pragma once

#include "ki/RenderClock.h"

#include "engine/PrepareContext.h"

//
// Context for doing updates, without rendering
//
struct UpdateContext final {
public:
    UpdateContext(
        const ki::RenderClock& clock);

    PrepareContext toPrepareContext() const;

public:
    const ki::RenderClock& m_clock;
};
