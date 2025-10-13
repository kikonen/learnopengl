#include "UpdateViewContext.h"

#include "util/thread.h"

UpdateViewContext::UpdateViewContext(
    Engine& engine,
    int width,
    int height)
    : BaseContext{ engine},
    m_resolution({ width, height })
{
    // #include "util/thread.h"
    ASSERT_RT();
}
