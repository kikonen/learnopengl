#include "UpdateContext.h"


UpdateContext::UpdateContext(
    const ki::RenderClock& clock,
    const Assets& assets,
    Registry* registry)
    : m_clock(clock),
    m_assets(assets),
    m_registry(registry)
{
}
