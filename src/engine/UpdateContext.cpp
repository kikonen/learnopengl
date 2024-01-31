#include "UpdateContext.h"

#include "engine/PrepareContext.h"

UpdateContext::UpdateContext(
    const ki::RenderClock& clock,
    Registry* registry)
    : m_clock(clock),
    m_registry(registry)
{
}

PrepareContext UpdateContext::toPrepareContext() const
{
    return {
        m_registry,
    };
}
