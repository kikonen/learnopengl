#include "InputContext.h"

#include "util/thread.h"

InputContext::InputContext(
    Engine& engine,
    const Input& input)
    : BaseContext{ engine },
    m_input{ input },
    m_inputState{ input.getState() }
{
    // #include "util/thread.h"
    ASSERT_RT();
}
