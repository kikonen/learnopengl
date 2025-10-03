#include "ActionContext.h"

namespace action
{
    ActionContext::ActionContext(
        Engine& engine,
        const Input& input)
        : InputContext{ engine, input }
    {
    }
}
