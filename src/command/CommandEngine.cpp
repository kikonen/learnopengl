#include "command/CommandEngine.h"

#include "model/Node.h"
#include "command/MoveNode.h"

#include "scene/RenderContext.h"
#include "scene/NodeRegistry.h"

CommandEngine::CommandEngine()
{}

void CommandEngine::executeCommands(const RenderContext& ctx)
{
    for (auto& cmd : m_commands) {
        const auto& node = ctx.registry.getNode(cmd->m_objectID);
        cmd->execute(ctx, *node);
    }
}

void CommandEngine::lua_moveTo(
    int objectID,
    float secs,
    std::array<float, 3> pos)
{
    m_commands.emplace_back(
        std::make_unique<MoveNode>(
            objectID,
            secs,
            glm::vec3{ pos[0], pos[1], pos[2] }));
}
