#include "SceneUpdater.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "ki/RenderClock.h"

#include "util/Log.h"

#include "event/Dispatcher.h"

#include "script/CommandEngine.h"

#include "audio/AudioEngine.h"

#include "physics/PhysicsEngine.h"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "component/ParticleGenerator.h"

namespace {
    size_t count = 0;
}

SceneUpdater::SceneUpdater(const Assets& assets,
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_registry(registry),
    m_alive(alive)
{}

SceneUpdater::~SceneUpdater()
{}

void SceneUpdater::destroy()
{
}

bool SceneUpdater::isRunning() const
{
    return m_running;
}

void SceneUpdater::prepare()
{
    m_registry->m_dispatcher->addListener(
        event::Type::scene_loaded,
        [this](const event::Event& e) {
            m_loaded = true;
        });
}

void SceneUpdater::start()
{
    auto th = std::thread{
        [this]() mutable {
            try {
                m_running = true;
                run();
                m_running = false;
            }
            catch (const std::exception& ex) {
                KI_CRITICAL(ex.what());
            }
            catch (...) {
                m_running = false;
            }
        }
    };
    th.detach();
}

void SceneUpdater::run()
{
    ki::RenderClock clock;

    //const int delay = (int)(1000.f / 60.f);
    const int delay = 10;

    auto prevLoopTime = std::chrono::system_clock::now();
    auto loopTime = std::chrono::system_clock::now();
    std::chrono::duration<float> elapsedDuration;

    while (*m_alive) {
        loopTime = std::chrono::system_clock::now();
        elapsedDuration = loopTime - prevLoopTime;

        clock.ts = static_cast<float>(glfwGetTime());
        clock.elapsedSecs = elapsedDuration.count();

        UpdateContext ctx(
            clock,
            m_assets,
            m_registry.get());

        update(ctx);

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

void SceneUpdater::update(const UpdateContext& ctx)
{
    m_registry->m_dispatcher->dispatchEvents();

    count++;
    //if (count < 100)
    {
        //std::cout << count << '\n';
        if (m_loaded) {
            m_registry->m_commandEngine->update(ctx);
        }

        if (auto root = m_registry->m_nodeRegistry->m_root) {
            root->update(ctx);
            m_registry->m_physicsEngine->update(ctx);
            m_registry->m_audioEngine->update(ctx);
        }
    }

    //for (auto& generator : m_particleGenerators) {
    //    generator->update(ctx);
    //}

    //if (m_particleSystem) {
    //    m_particleSystem->update(ctx);
    //}

    m_registry->update(ctx);
}
