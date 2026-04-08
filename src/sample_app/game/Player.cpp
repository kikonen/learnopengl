#include "Player.h"

#include "engine/PrepareContext.h"

#include "registry/Registry.h"

#include "action/ActionContext.h"
#include "action/Shoot.h"

namespace game
{
    Player::Player()
    {
    }

    Player::~Player()
    {
    }

    void Player::prepare(const PrepareContext& ctx)
    {
        m_engine = &ctx.getEngine();
        auto* registry = ctx.getRegistry();

        m_listen_action_game_shoot.listen(
            event::Type::action_game_shoot,
            registry->m_dispatcherWorker,
            [this](const event::Event& e)
        {
            const auto& action = e.body.action;
            const auto handle = pool::NodeHandle::toHandle(action.target);

            action::ActionContext actionCtx{
                *m_engine,
                handle,
                action.pos,
                action.dir
            };

            action::Shoot handler;
            handler.handle(actionCtx);
        });
    }
}
