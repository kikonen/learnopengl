#pragma once

#include <memory>

#include "asset/Assets.h"

class UpdateContext;
class Registry;

class SceneUpdater
{
public:
    SceneUpdater(
        const Assets& assets,
        std::shared_ptr<Registry> registry,
        std::shared_ptr<std::atomic<bool>> alive);

    ~SceneUpdater();

    void destroy();

    void prepare();
    void start();
    void run();

    void update(const UpdateContext& ctx);

private:
    const Assets& m_assets;

    bool m_loaded{ false };

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::shared_ptr<Registry> m_registry;
};
