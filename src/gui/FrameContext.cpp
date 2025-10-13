#include "FrameContext.h"

#include "util/thread.h"

namespace gui
{
    FrameContext::FrameContext(
        Engine& engine)
        : BaseContext{ engine }
    {
        // #include "util/thread.h"
        ASSERT_RT();
    }
}
