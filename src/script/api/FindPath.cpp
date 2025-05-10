#include "FindPath.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "nav/Path.h"
#include "nav/Query.h"
#include "nav/NavigationSystem.h"

#include "script/ScriptSystem.h"

#include "registry/Registry.h"

namespace script
{
    FindPath::FindPath(
        pool::NodeHandle handle,
        const glm::vec3& startPos,
        const glm::vec3& endPos,
        int maxPath,
        const std::function<void(int cid, const nav::Path&)>& callback) noexcept
        : NodeCommand(handle, 0, false),
        m_startPos{ startPos },
        m_endPos{ endPos },
        m_maxPath{ maxPath },
        m_callback{ callback }
    {
    }

    void FindPath::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            const auto& state = getNode()->getState();

            nav::Query query{ m_startPos, m_endPos, m_maxPath };

            nav::Path path = nav::NavigationSystem::get().findPath(query);

            if (!path.empty()) {
                m_callback(m_id, path);
            }
        }
    }
}
