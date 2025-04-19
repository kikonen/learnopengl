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
        const uint32_t categoryMask,
        const uint32_t collisionMask,
        const bool self,
        const sol::function callback) noexcept
        : NodeCommand(handle, 0, false),
        m_dirs{ dirs },
        m_categoryMask{ categoryMask },
        m_collisionMask{ collisionMask },
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

            const auto& hits = physics::PhysicsEngine::get().rayCastClosestToMultiple(
                state.getWorldPosition(),
                m_dirs,
                400.f,
                m_categoryMask,
                m_collisionMask,
                m_handle);

            if (!hits.empty()) {
                auto& se = script::ScriptEngine::get();
                sol::table args = se.getLua().create_table();

                for (const auto& hit : hits) {
                    auto* node = hit.handle.toNode();

                    args["data"] = hit;

                    se.invokeNodeFunction(
                        getNode(),
                        m_self,
                        m_callback,
                        args);
                }
            }
        }
    }
}
