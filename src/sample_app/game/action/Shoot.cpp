#include "Shoot.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "util/glm_format.h"

#include "ki/RenderClock.h"

#include "debug/DebugContext.h"

#include "model/Node.h"

#include "decal/DecalSystem.h"
#include "decal/DecalRegistry.h"

#include "script/CommandEngine.h"
#include "script/command/RayCast.h"

#include "physics/PhysicsSystem.h"
#include "physics/RayHit.h"
#include "physics/physics_util.h"

#include "scene/Scene.h"

namespace
{
    constexpr float HIT_RATE = 0.01f;
    float g_hitElapsed = 0.f;
}

namespace action
{
    void Shoot::handle(const ActionContext& ctx)
    {
        {
            const auto& dbg = debug::DebugContext::get();

            const auto handle = ctx.m_handle;
            const glm::vec3 origin{ctx.m_pos };
            const glm::vec3 dir{ ctx.m_dir };

            //const auto& hits = physics::PhysicsSystem::get().rayCast(
            //    startPos,
            //    dir,
            //    400.f,
            //    physics::mask(physics::Category::ray_player_fire),
            //    physics::mask(physics::Category::npc, physics::Category::prop),
            //    //physics::mask(physics::Category::all),
            //    player->toHandle(),
            //    true);

            auto callback = [this, handle](int cid, const physics::RayHit& hits) {
                shootCallback(handle, hits);
                };

            auto& commandEngine = script::CommandEngine::get();
            commandEngine.addCommand(
                0,
                script::RayCast{
                    handle,
                    origin,
                    dir,
                    400.f,
                    physics::mask(physics::Category::npc, physics::Category::prop, physics::Category::terrain),
                    false,
                    callback
                });

            g_hitElapsed += ctx.getClock().elapsedSecs;
        }
    }

    void Shoot::shootCallback(
        const pool::NodeHandle& playerHandle,
        const physics::RayHit& hit
    )
    {
        auto* player = playerHandle.toNode();
        if (!player) return;

        {
            const auto& dbg = debug::DebugContext::get();

            if (hit.isHit && g_hitElapsed >= HIT_RATE) {
                g_hitElapsed -= HIT_RATE;

                {
                    auto* node = hit.handle.toNode();
                    KI_INFO_OUT(fmt::format(
                        "SCREEN_HIT: node={}, pos={}, normal={}, depth={}",
                        node ? node->getName() : "N/A",
                        hit.pos,
                        hit.normal,
                        hit.depth));

                    auto sid = dbg.m_decalId;
                    auto df = decal::DecalRegistry::get().getDecal(sid);
                    KI_INFO_OUT(fmt::format("DECAL: name={}, valid={}", sid.str(), df ? true : false));

                    auto decal = df.createForHit(hit.handle, hit.pos, glm::normalize(hit.normal));

                    decal::DecalSystem::get().addDecal(decal);

                    KI_INFO_OUT(fmt::format(
                        "DECAL: node={}, pos={}, normal={}",
                        node ? node->getName() : "N/A",
                        decal.m_position,
                        decal.m_normal));
                }
            }
        }
    }
}
