#pragma once

#include <vector>

#include "asset/Assets.h"
#include "Command.h"

namespace backend {
    class RenderSystem {
    public:
        RenderSystem(const Assets& assets);

        void init();
        void shutdown();

        void render();
        void addCommand(const backend::Command& command);

    public:

    private:
        const Assets& m_assets;
        std::vector<backend::Command> m_pending;
    };
}
