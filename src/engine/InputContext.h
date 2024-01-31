#pragma once

#include "ki/RenderClock.h"

#include "gui/Input.h"

#include "engine/PrepareContext.h"

class Registry;

//
// Context for doing updates, without rendering
//
struct InputContext final {
public:
    InputContext(
        const ki::RenderClock& clock,
        Registry* registry,
        const Input* const input);

    PrepareContext toPrepareContext() const;

public:
    const ki::RenderClock& m_clock;

    Registry* const m_registry;

    const Input* const m_input;
};
