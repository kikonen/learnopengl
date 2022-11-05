#include "RenderSystem.h"

#include "ki/GL.h"

namespace backend {
    RenderSystem::RenderSystem(const Assets& assets)
        : m_assets(assets)
    {
    }

    void RenderSystem::init()
    {
    }

    void RenderSystem::shutdown()
    {
    }

    void RenderSystem::render()
    {
        for (const auto& cmd : m_pending) {
        }
    }

    void RenderSystem::addCommand(const backend::Command& command)
    {
        m_pending.emplace_back(command);
    }
}
