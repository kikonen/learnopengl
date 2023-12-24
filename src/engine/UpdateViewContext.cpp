#include "UpdateViewContext.h"


UpdateViewContext::UpdateViewContext(
    const ki::RenderClock& clock,
    const Assets& assets,
    Registry* registry,
    int width,
    int height)
    : m_clock(clock),
    m_assets(assets),
    m_registry(registry),
    m_resolution({ width, height })
{
}
