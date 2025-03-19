#include "LuaPhysics.h"

#include "physics/PhysicsEngine.h"


namespace script
{
    // https://thephd.dev/sol3-feature-complete
    void LuaPhysics::bind(sol::state& lua)
    {
        //auto* player = m_currentScene->getActiveNode();
        //if (!player) return;

        //{
        //    const auto& dbg = render::DebugContext::get();

        //    glm::vec2 screenPos{ m_window->m_input->mouseX, m_window->m_input->mouseY };

        //    const auto startPos = ctx.unproject(screenPos, .01f);
        //    const auto endPos = ctx.unproject(screenPos, .8f);
        //    const auto dir = glm::normalize(endPos - startPos);

        //    const auto& hits = physics::PhysicsEngine::get().rayCast(
        //        startPos,
        //        dir,
        //        400.f,
        //        physics::mask(physics::Category::ray_player_fire),
        //        physics::mask(physics::Category::npc, physics::Category::prop),
        //        //physics::mask(physics::Category::all),
        //        player->toHandle(),
        //        true);
        //}
    }
}
