#pragma once

#include <memory>

#include "glm/glm.hpp"

class AsyncLoader;

class Engine;

class TestSceneSetup final
{
public:
    TestSceneSetup(
        Engine& engine,
        std::shared_ptr<std::atomic<bool>> alive,
        std::shared_ptr<AsyncLoader> asyncLoader);

    void setup();

private:
    void setupEffectExplosion();

    void setupViewport1();

private:
    Engine& m_engine;
    std::shared_ptr<std::atomic<bool>> m_alive;

    std::shared_ptr<AsyncLoader> m_asyncLoader;
};
