#include "UpdateViewContext.h"

#include "asset/Assets.h"

UpdateViewContext::UpdateViewContext(
    const ki::RenderClock& clock,
    Registry* registry,
    int width,
    int height,
    const render::DebugContext* dbg)
    : m_assets{ Assets::get() },
    m_clock(clock),
    m_registry(registry),
    m_resolution({ width, height }),
    m_dbg{ dbg }
{
}
