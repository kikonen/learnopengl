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
        const glm::vec3& dir,
        const bool self,
        const sol::function callback) noexcept
        : NodeCommand(handle, 0, false),
        m_dir{ dir },
        m_self{ self },
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

            const auto& hits = physics::PhysicsEngine::get().rayCast(
                state.getWorldPosition(),
                m_dir,
                400.f,
                physics::mask(physics::Category::ray_npc_fire),
                physics::mask(physics::Category::player),
                m_handle,
                true);

            if (!hits.empty()) {
                auto& se = script::ScriptEngine::get();
                const auto& hit = hits[0];
                auto* node = hit.handle.toNode();

                sol::table args = se.getLua().create_table();
                args["hit"] = hit;
                args["hit_name"] = node->getName();

                se.invokeNodeFunction(
                    getNode(),
                    m_self,
                    m_callback,
                    args);
            }
        }
    }
}
