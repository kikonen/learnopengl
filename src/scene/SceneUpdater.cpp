#include "SceneUpdater.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "ki/RenderClock.h"
#include "ki/Timer.h"

#include "util/Log.h"

#include "event/Dispatcher.h"

#include "script/CommandEngine.h"

#include "audio/AudioEngine.h"

#include "physics/PhysicsEngine.h"

#include "engine/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "component/ParticleGenerator.h"

#define KI_TIMER(x)

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
{
    KI_INFO("SCENE_UPDATER: destroy");
}

void SceneUpdater::destroy()
{
}

bool SceneUpdater::isRunning() const
{
    return m_running;
}

void SceneUpdater::prepare()
{
    m_registry->prepareWorker();

    auto* dispatcher = m_registry->m_dispatcher;

    dispatcher->addListener(
        event::Type::scene_loaded,
        [this](const event::Event& e) {
            m_loaded = true;
            this->m_registry->m_physicsEngine->setEnabled(true);

            // NOTE KI trigger UI sidew update *after* all worker side processing done
            {
                event::Event evt { event::Type::scene_loaded };
                m_registry->m_dispatcherView->send(evt);
            }
        });

    dispatcher->addListener(
        event::Type::node_added,
        [this](const event::Event& e) {
            this->handleNodeAdded(e.body.node.target);
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

    prepare();

    //const int delay = (int)(1000.f / 60.f);
    const int delay = 5;

    auto prevLoopTime = std::chrono::system_clock::now();
    auto loopTime = std::chrono::system_clock::now();
    std::chrono::duration<float> elapsedDuration;

    while (*m_alive) {
        loopTime = std::chrono::system_clock::now();
        elapsedDuration = loopTime - prevLoopTime;
        prevLoopTime = loopTime;

        auto ts = duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        );
        clock.ts = static_cast<float>(ts.count());
        clock.elapsedSecs = elapsedDuration.count();

        //std::cout << "elapsed=" << clock.elapsedSecs << '\n';

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
    KI_TIMER("loop    ");
    {
        KI_TIMER("event   ");
        m_registry->m_dispatcher->dispatchEvents();
    }

    count++;
    //if (count < 100)
    {
        //std::cout << count << '\n';
        if (m_loaded) {
            KI_TIMER("command ");
            m_registry->m_commandEngine->update(ctx);
        }

        if (auto root = m_registry->m_nodeRegistry->m_root) {
            {
                KI_TIMER("model   ");
                root->update(ctx);
            }
            if (m_loaded) {
                {
                    KI_TIMER("bounds  ");
                    m_registry->m_physicsEngine->updateBounds(ctx);
                }
                {
                    KI_TIMER("physics ");
                    m_registry->m_physicsEngine->update(ctx);
                }
                {
                    KI_TIMER("audio   ");
                    m_registry->m_audioEngine->update(ctx);
                }
            }
        }
    }

    //for (auto& generator : m_particleGenerators) {
    //    generator->update(ctx);
    //}

    //if (m_particleSystem) {
    //    m_particleSystem->update(ctx);
    //}

    {
        KI_TIMER("registry");
        m_registry->update(ctx);
    }
}

void SceneUpdater::handleNodeAdded(Node* node)
{
    m_registry->m_physicsEngine->handleNodeAdded(node);
}
