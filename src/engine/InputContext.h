#pragma once

#include "ki/RenderClock.h"

#include "gui/Input.h"

#include "engine/PrepareContext.h"

class Assets;
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

    inline bool allowMouse() const noexcept
    {
        return m_input->allowMouse();
    }

    inline bool allowKeyboard() const noexcept
    {
        return m_input->allowKeyboard();
    }

public:
    const Assets& m_assets;
    const ki::RenderClock& m_clock;

    Registry* const m_registry;

    const Input* const m_input;
};
