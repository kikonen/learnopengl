#include "RayCast.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "physics/physics_util.h"
#include "physics/RayHit.h"
#include "physics/PhysicsEngine.h"

#include "script/ScriptEngine.h"

#include "registry/Registry.h"

namespace script
{
    RayCast::RayCast(
        pool::NodeHandle handle,
        const std::vector<glm::vec3>& dirs,
        float length,
        const uint32_t categoryMask,
        const uint32_t collisionMask,
        const std::function<void(const std::vector<physics::RayHit>&)>& callback) noexcept
        : NodeCommand(handle, 0, false),
        m_dirs{ dirs },
        m_length{ length },
        m_categoryMask{ categoryMask },
        m_collisionMask{ collisionMask },
        m_callback{ callback }
    {
    }

    void RayCast::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            const auto& state = getNode()->getState();

            const auto& hits = physics::PhysicsEngine::get().rayCastClosestToMultiple(
                state.getWorldPosition(),
                m_dirs,
                m_length,
                m_categoryMask,
                m_collisionMask,
                m_handle);

            if (!hits.empty()) {
                m_callback(hits);
            }
        }
    }
}
