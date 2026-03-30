#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "util/Ref.h"

class AsyncLoader;

class Engine;

class TestSceneSetup final
{
public:
    TestSceneSetup(
        Engine& engine,
        const std::shared_ptr<std::atomic_bool>& alive,
        const util::Ref<AsyncLoader>& asyncLoader);

    void setup();

private:
    void setupEffectExplosion();

    void setupViewport1();

private:
    Engine& m_engine;
    std::shared_ptr<std::atomic_bool> m_alive;

    util::Ref<AsyncLoader> m_asyncLoader;

    std::shared_ptr<std::atomic<int>> m_runningCount;
};
