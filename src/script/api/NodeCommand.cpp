#include "NodeCommand.h"

#include "model/Node.h"

namespace script
{
    NodeCommand::NodeCommand(
        pool::NodeHandle handle,
        float duration,
        bool relative) noexcept
        : Command{ duration },
        m_handle{ handle },
        m_relative{ relative }
    {
    }

    void NodeCommand::bind(const UpdateContext& ctx) noexcept
    {
        Command::bind(ctx);
        m_finished = !m_handle.toNode();
    }
}
