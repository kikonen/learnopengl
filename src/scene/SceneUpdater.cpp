#include "SceneUpdater.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "ki/RenderClock.h"
#include "ki/Timer.h"
#include "ki/FpsCounter.h"

#include "model/Node.h"

#include "pool/NodeHandle.h"

#include "util/thread.h"
#include "util/Log.h"

#include "event/Dispatcher.h"

#include "script/CommandEngine.h"

#include "audio/AudioEngine.h"

#include "engine/UpdateContext.h"

#include "physics/PhysicsEngine.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/NodeSnapshotRegistry.h"

#include "registry/SnapshotRegistry_impl.h"


#define KI_TIMER(x)

namespace {
    size_t count = 0;
}

SceneUpdater::SceneUpdater(
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_registry(registry),
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
    std::lock_guard lock(m_prepareLock);

    m_registry->prepareWT();

    auto* dispatcher = m_registry->m_dispatcherWorker;

    dispatcher->addListener(
        event::Type::scene_loaded,
        [this](const event::Event& e) {
            m_loaded = true;

            // NOTE KI trigger UI sidew update *after* all worker side processing done
            {
                event::Event evt { event::Type::scene_loaded };
                m_registry->m_dispatcherView->send(evt);
            }
        });

    dispatcher->addListener(
        event::Type::node_added,
        [this](const event::Event& e) {
            auto* node = pool::NodeHandle::toNode(e.body.node.target);
            this->handleNodeAdded(node);

            {
                event::Event evt { event::Type::node_added };
                evt.body.node = {
                    .target = e.body.node.target,
                };
                m_registry->m_dispatcherView->send(evt);
            }
        });

    dispatcher->addListener(
        event::Type::physics_add,
        [this](const event::Event& e) {
            auto& data = e.blob->body.physics;
            auto& pe = physics::PhysicsEngine::get();
            auto* node = pool::NodeHandle::toNode(e.body.node.target);

            {
                physics::Object obj{};
                obj.m_update = data.update;
                obj.m_body = data.body;
                obj.m_geom = data.geom;
                obj.m_nodeHandle = node->toHandle();
                obj.m_nodeSnapshotIndex = node->m_snapshotIndex;

                auto [physicsId, snapshotIndex] = pe.registerObject(obj);
                node->m_physicsId = physicsId;
                node->m_objectSnapshotIndex = snapshotIndex;
            }
        });

    m_prepared = true;
}

void SceneUpdater::start()
{
    auto th = std::thread{
        [this]() mutable {
            try {
                m_running = true;
                util::markWorkerThread();
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

    KI_INFO(fmt::format("WT: started - worker={}", util::isWorkerThread()));

    prepare();

    //const int delay = (int)(1000.f / 60.f);
    const int delay = 5;

    ki::FpsCounter fpsCounter;

    auto prevLoopTime = std::chrono::system_clock::now();
    auto loopTime = std::chrono::system_clock::now();
    std::chrono::duration<float> elapsedDuration;

    while (*m_alive) {
        fpsCounter.startFame();

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
            m_registry.get());

        update(ctx);

        fpsCounter.endFame(clock.elapsedSecs);

        if (fpsCounter.isUpdate())
        {
            KI_INFO(fmt::format(
                "{} - nodes={}",
                fpsCounter.formatSummary("WT"),
                ctx.m_registry->m_nodeRegistry->getNodeCount()));
        }

        if (!script::CommandEngine::get().hasPending()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    }

    KI_INFO(fmt::format("WT: stopped - worker={}", util::isWorkerThread()));
}

void SceneUpdater::update(const UpdateContext& ctx)
{
    KI_TIMER("loop    ");
    {
        KI_TIMER("event   ");
        m_registry->m_dispatcherWorker->dispatchEvents();
    }

    count++;
    //if (count < 100)
    {
        //std::cout << count << '\n';
        if (m_loaded) {
            KI_TIMER("command ");
            script::CommandEngine::get().update(ctx);
        }

        {
            KI_TIMER("registry");
            m_registry->updateWT(ctx);
        }

        if (m_loaded) {
            {
                KI_TIMER("node    ");
                NodeRegistry::get().updateWT(ctx);
            }
            {
                KI_TIMER("audio   ");
                audio::AudioEngine::get().update(ctx);
            }
        }
    }

    // NOTE KI sync to RT
    m_registry->m_pendingSnapshotRegistry->copyFrom(
        m_registry->m_workerSnapshotRegistry,
        0, -1);

    if (m_loaded) {
        physics::PhysicsEngine::get().setEnabled(true);
    }
}

void SceneUpdater::handleNodeAdded(Node* node)
{
    if (!node) return;

    physics::PhysicsEngine::get().registerBoundsNode(node);
}
