#include "UpdateViewContext.h"

#include "asset/Assets.h"

#include "kigl/GLState.h"

UpdateViewContext::UpdateViewContext(
    const ki::RenderClock& clock,
    Registry* registry,
    int width,
    int height,
    const debug::DebugContext& dbg)
    : m_assets{ Assets::get() },
    m_clock(clock),
    m_state{ kigl::GLState::get() },
    m_registry(registry),
    m_resolution({ width, height }),
    m_dbg{ dbg }
{
}
