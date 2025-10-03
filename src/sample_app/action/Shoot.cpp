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
        Scene* scene = nullptr;

        const auto& input = ctx.getInput();
        auto* player = scene->getActiveNode();
        if (!player) return;

        {
            const auto& dbg = debug::DebugContext::get();

            glm::vec2 screenPos{ input.mouseX, input.mouseY };

            //const auto startPos = ctx.unproject(screenPos, .01f);
            //const auto endPos = ctx.unproject(screenPos, .8f);
            //const auto dir = glm::normalize(endPos - startPos);

            const glm::vec3 startPos{ 0.f };
            const glm::vec3 endPos{ 0.f };
            const glm::vec3 dir{ 0.f };

            //const auto& hits = physics::PhysicsSystem::get().rayCast(
            //    startPos,
            //    dir,
            //    400.f,
            //    physics::mask(physics::Category::ray_player_fire),
            //    physics::mask(physics::Category::npc, physics::Category::prop),
            //    //physics::mask(physics::Category::all),
            //    player->toHandle(),
            //    true);

            auto callback = [this](int cid, const physics::RayHit& hits) {
                shootCallback(hits);
                };

            auto& commandEngine = script::CommandEngine::get();
            commandEngine.addCommand(
                0,
                script::RayCast{
                    player->toHandle(),
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
        const physics::RayHit& hit
    )
    {
        Scene* scene = nullptr;
        if (!scene) return;

        auto* player = scene->getActiveNode();
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
