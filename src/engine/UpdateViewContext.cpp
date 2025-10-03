#include "UpdateViewContext.h"

UpdateViewContext::UpdateViewContext(
    Engine& engine,
    int width,
    int height)
    : BaseContext{ engine},
    m_resolution({ width, height })
{}
