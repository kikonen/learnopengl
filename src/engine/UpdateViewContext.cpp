#include "UpdateViewContext.h"


UpdateViewContext::UpdateViewContext(
    const ki::RenderClock& clock,
    Registry* registry,
    int width,
    int height)
    : m_clock(clock),
    m_registry(registry),
    m_resolution({ width, height })
{
}
