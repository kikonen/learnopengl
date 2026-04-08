#pragma once

#include "util/Ref.h"

#include "event/Listen.h"

struct PrepareContext;
class Engine;

namespace game
{
    class Player : public util::RefCounted<true>
    {
    public:
        Player();
        ~Player();

        void prepare(const PrepareContext& ctx);

    private:
        event::Listen m_listen_action_game_shoot;
        Engine* m_engine;
    };
}
