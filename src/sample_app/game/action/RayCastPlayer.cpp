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
        {
            auto* player = ctx.m_handle.toNode();
            if (player) {
                const auto& state = player->getState();
                const glm::vec3 startPos{ state.getWorldPosition() };
                const glm::vec3 dir{ state.getViewFront() };

                const auto& hit = physics::PhysicsSystem::get().rayCastClosest(
                    startPos,
                    dir,
                    100.f,
                    physics::mask(physics::Category::npc),
                    ctx.m_handle);

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
        }

        {
            const glm::vec3 startPos{ ctx.m_pos };
            const glm::vec3 dir{ ctx.m_dir };

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
                        startPos + dir * 10.f
                    });
                g_rayMarkers.push_back(cmdId);
            }

            const auto& hit = physics::PhysicsSystem::get().rayCastClosest(
                startPos,
                dir,
                400.f,
                physics::mask(physics::Category::npc, physics::Category::prop),
                //physics::mask(physics::Category::all),
                ctx.m_handle);
        }
    }


}
