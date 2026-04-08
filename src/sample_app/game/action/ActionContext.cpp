#include "ActionContext.h"

namespace action
{
    ActionContext::ActionContext(
        Engine& engine,
        const pool::NodeHandle& handle,
        const glm::vec3& pos,
        const glm::vec3& dir)
        : BaseContext{ engine },
        m_handle{ handle },
        m_pos{ pos },
        m_dir{ dir }
    {
    }
}
