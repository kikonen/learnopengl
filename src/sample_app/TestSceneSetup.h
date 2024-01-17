#pragma once

#include <memory>

#include "glm/glm.hpp"

#include "asset/Assets.h"

class AsyncLoader;

class Registry;

class TestSceneSetup final
{
public:
    TestSceneSetup(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive,
        std::shared_ptr<AsyncLoader> asyncLoader);

    void setup(
        std::shared_ptr<Registry> registry);

private:
    void setupEffectExplosion();

    void setupViewport1();
private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::shared_ptr<AsyncLoader> m_asyncLoader;

    std::shared_ptr<Registry> m_registry{ nullptr };
};
