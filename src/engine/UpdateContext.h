#pragma once

#include "BaseContext.h"

//
// Context for doing updates, without rendering
//
struct UpdateContext final : BaseContext
{
public:
    UpdateContext(
        Engine& engine,
        const ki::RenderClock& clock);

    const ki::RenderClock& getClock() const noexcept
    {
        return m_clock;
    }

private:
    const ki::RenderClock& m_clock;
};
