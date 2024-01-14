#include "UpdateContext.h"

#include "engine/PrepareContext.h"

UpdateContext::UpdateContext(
    const ki::RenderClock& clock,
    const Assets& assets,
    Registry* registry)
    : m_clock(clock),
    m_assets(assets),
    m_registry(registry)
{
}

PrepareContext UpdateContext::toPrepareContext() const
{
    return {
        m_assets,
        m_registry,
    };
}
