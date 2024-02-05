#include "UpdateViewContext.h"


UpdateViewContext::UpdateViewContext(
    const ki::RenderClock& clock,
    int width,
    int height)
    : m_clock(clock),
    m_resolution({ width, height })
{
}
