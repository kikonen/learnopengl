#include "UpdateContext.h"

#include "asset/Assets.h"

#include "engine/PrepareContext.h"

UpdateContext::UpdateContext(
    const ki::RenderClock& clock,
    Registry* registry)
    : m_assets{ Assets::get() },
    m_clock(clock),
    m_registry(registry)
{
}

PrepareContext UpdateContext::toPrepareContext() const
{
    return {
        m_registry,
    };
}
