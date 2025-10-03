#include "InputContext.h"

InputContext::InputContext(
    Engine& engine,
    const Input& input)
    : BaseContext{ engine },
    m_input{ input },
    m_inputState{ input.getState() }
{
}
