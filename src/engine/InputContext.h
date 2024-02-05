#pragma once

#include "ki/RenderClock.h"

#include "gui/Input.h"

#include "engine/PrepareContext.h"

//
// Context for doing updates, without rendering
//
struct InputContext final {
public:
    InputContext(
        const ki::RenderClock& clock,
        const Input* const input);

    PrepareContext toPrepareContext() const;

public:
    const ki::RenderClock& m_clock;

    const Input* const m_input;
};
