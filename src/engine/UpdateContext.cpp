#include "UpdateContext.h"

#include "engine/PrepareContext.h"

UpdateContext::UpdateContext(
    const ki::RenderClock& clock)
    : m_clock(clock)
{
}

PrepareContext UpdateContext::toPrepareContext() const
{
    return {
    };
}
