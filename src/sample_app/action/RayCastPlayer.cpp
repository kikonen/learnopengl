#include "RayCastPlayer.h"

#include <fmt/format.h>

#include "ki/sid.h"

#include "util/Log.h"
#include "util/glm_format.h"

#include "model/Node.h"

#include "physics/PhysicsSystem.h"
#include "physics/RayHit.h"
#include "physics/physics_util.h"

#include "script/size.h"
#include "script/CommandEngine.h"
#include "script/command/Wait.h"
#include "script/command/MoveNode.h"
#include "script/command/RayCast.h"

#include "scene/Scene.h"

#include "ActionContext.h"

namespace {
    std::vector<script::command_id> g_rayMarkers;
}

namespace action
{
    void RayCastPlayer::handle(const ActionContext& ctx)
    {
        auto* scene = ctx.getScene();
        if (!scene) return;

        auto* player = scene->getActiveNode();
        if (!player) return;

        {
            const auto* snapshot = player->getSnapshotRT();
            if (!snapshot) return;

            const auto& hit = physics::PhysicsSystem::get().rayCastClosest(
                snapshot->getWorldPosition(),
                snapshot->getViewFront(),
                100.f,
                physics::mask(physics::Category::npc),
                player->toHandle());

            if (hit.isHit) {
                auto* node = hit.handle.toNode();

                KI_INFO_OUT(fmt::format(
                    "PLAYER_HIT: node={}, pos={}, normal={}, depth={}",
                    node ? node->getName() : "N/A",
                    hit.pos,
                    hit.normal,
                    hit.depth));
            }
        }

        {
            const auto& input = ctx.getInput();

            glm::vec2 screenPos{ input.mouseX, input.mouseY };

            //const auto startPos = ctx.unproject(screenPos, .01f);
            //const auto endPos = ctx.unproject(screenPos, .8f);
            //const auto dir = glm::normalize(endPos - startPos);

            const glm::vec3 startPos{ 0.f };
            const glm::vec3 endPos{ 0.f };
            const glm::vec3 dir{ 0.f };

            KI_INFO_OUT(fmt::format(
                "UNPROJECT: screenPos={}, z0={}, z1={}",
                screenPos, startPos, endPos));

            auto greenBall = pool::NodeHandle::toNode(SID("green_ball"));
            auto redBall = pool::NodeHandle::toNode(SID("red_ball"));

            for (auto& cmdId : g_rayMarkers) {
                script::CommandEngine::get().cancelCommand(cmdId);
            }
            g_rayMarkers.clear();

            if (greenBall) {
                auto cmdId = 0;
                for (int i = 0; i < 5; i++) {
                    cmdId = script::CommandEngine::get().addCommand(
                        cmdId,
                        script::MoveNode{
                            greenBall->toHandle(),
                            0.f,
                            false,
                            startPos
                        });
                    g_rayMarkers.push_back(cmdId);

                    if (i == 0) {
                        cmdId = script::CommandEngine::get().addCommand(
                            cmdId,
                            script::Wait{
                                2.f
                            });
                        g_rayMarkers.push_back(cmdId);
                    }

                    cmdId = script::CommandEngine::get().addCommand(
                        cmdId,
                        script::MoveNode{
                            greenBall->toHandle(),
                            2.f,
                            true,
                            dir * 5.f
                        });
                    g_rayMarkers.push_back(cmdId);
                }
                {
                    cmdId = script::CommandEngine::get().addCommand(
                        cmdId,
                        script::MoveNode{
                            greenBall->toHandle(),
                            0.f,
                            false,
                            startPos
                        });
                    g_rayMarkers.push_back(cmdId);
                }
            }
            if (redBall) {
                auto cmdId = script::CommandEngine::get().addCommand(
                    0,
                    script::MoveNode{
                        redBall->toHandle(),
                        0.f,
                        false,
                        endPos
                    });
                g_rayMarkers.push_back(cmdId);
            }

            const auto& hit = physics::PhysicsSystem::get().rayCastClosest(
                startPos,
                dir,
                400.f,
                physics::mask(physics::Category::npc, physics::Category::prop),
                //physics::mask(physics::Category::all),
                player->toHandle());
        }
    }


}
