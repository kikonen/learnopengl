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
        const glm::vec3& dir,
        float length,
        const uint32_t collisionMask,
        bool notifyMiss,
        const std::function<void(int cid, const physics::RayHit&)>& callback) noexcept
        : NodeCommand(handle, 0, false),
        m_dir{ dir },
        m_length{ length },
        m_collisionMask{ collisionMask },
        m_notifyMiss{ notifyMiss },
        m_callback{ callback }
    {
    }

    void RayCast::execute(
        const UpdateContext& ctx) noexcept
    {
        m_elapsedTime += ctx.m_clock.elapsedSecs;

        m_finished = m_elapsedTime >= m_duration;
        if (m_finished) {
            auto* node = getNode();
            if (!node) return;

            const auto& state = node->getState();

            const auto& hit = physics::PhysicsSystem::get().rayCastClosest(
                state.getWorldPosition(),
                m_dir,
                m_length,
                m_collisionMask,
                m_handle);

            if (hit.isHit || m_notifyMiss) {
                m_callback(m_id, hit);
            }
        }
    }
}
