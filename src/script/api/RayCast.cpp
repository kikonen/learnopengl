#include "RayCast.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "physics/physics_util.h"
#include "physics/RayHit.h"
#include "physics/PhysicsSystem.h"

#include "script/ScriptSystem.h"

#include "registry/Registry.h"

namespace script
{
    RayCast::RayCast(
        pool::NodeHandle handle,
        const std::vector<glm::vec3>& dirs,
        float length,
        const uint32_t collisionMask,
        const std::function<void(const std::vector<physics::RayHit>&)>& callback) noexcept
        : NodeCommand(handle, 0, false),
        m_dirs{ dirs },
        m_length{ length },
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

            const auto& hits = physics::PhysicsSystem::get().rayCastClosestToMultiple(
                state.getWorldPosition(),
                m_dirs,
                m_length,
                m_collisionMask,
                m_handle);

            if (!hits.empty()) {
                m_callback(hits);
            }
        }
    }
}
