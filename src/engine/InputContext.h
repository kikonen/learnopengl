#pragma once

#include <memory>

#include "gui/Input.h"

#include "engine/BaseContext.h"

struct InputContext : BaseContext
{
public:
    InputContext(
        Engine& engine,
        const Input& input);

    inline bool allowMouse() const noexcept
    {
        return m_input.allowMouse();
    }

    inline bool allowKeyboard() const noexcept
    {
        return m_input.allowKeyboard();
    }

    const Input& getInput() const noexcept
    {
        return m_input;
    }

    const InputState& getInputState() const noexcept
    {
        return m_inputState;
    }

protected:
    const Input& m_input;
    const InputState m_inputState;
};
