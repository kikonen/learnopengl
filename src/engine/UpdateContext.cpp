#include "UpdateContext.h"

UpdateContext::UpdateContext(
    Engine& engine,
    const ki::RenderClock& clock)
    : BaseContext{ engine },
    m_clock { clock }
{ }
