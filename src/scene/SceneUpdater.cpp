#include "SceneUpdater.h"

#include "ki/Timer.h"

#include "asset/Assets.h"

#include "model/Node.h"

#include "pool/NodeHandle.h"

#include "util/Log.h"

#include "event/Dispatcher.h"

#include "script/CommandEngine.h"

#include "audio/AudioEngine.h"

#include "animation/AnimationSystem.h"
#include "decal/DecalSystem.h"

#include "engine/UpdateContext.h"

#include "physics/PhysicsEngine.h"

#include "script/ScriptEngine.h"

#include "registry/Registry.h"
#include "registry/ControllerRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/NodeSnapshotRegistry.h"

#include "registry/SnapshotRegistry_impl.h"

#define KI_TIMER(x)

namespace {
    size_t g_count = 0;
}

SceneUpdater::SceneUpdater(
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : Updater("WT", 5, registry, alive)
{}

void SceneUpdater::prepare()
{
    if (m_prepared) return;
    Updater::prepare();

    const auto& assets = Assets::get();

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

    if (assets.useScript)
    {
        dispatcher->addListener(
            event::Type::script_bind,
            [this](const event::Event& e) {
                auto& data = e.body.script;
                auto handle = pool::NodeHandle::toHandle(data.target);
                script::ScriptEngine::get().bindNodeScript(handle, data.id);
            });

        dispatcher->addListener(
            event::Type::script_run,
            [this](const event::Event& e) {
                auto& data = e.body.script;
                if (data.target) {
                    if (auto* node = pool::NodeHandle::toNode(data.target)) {
                        script::ScriptEngine::get().runNodeScript(node, data.id);
                    }
                }
                else {
                    script::ScriptEngine::get().runGlobalScript(data.id);
                }
            });
    }

    m_prepared = true;
}

uint32_t SceneUpdater::getActiveCount() const noexcept
{
    return m_registry->m_nodeRegistry->getNodeCount();
}

void SceneUpdater::update(const UpdateContext& ctx)
{
    KI_TIMER("[loop]  ");

    auto& nodeRegistry = NodeRegistry::get();
    auto& physicsEngine = physics::PhysicsEngine::get();

    {
        KI_TIMER("event   ");
        m_registry->m_dispatcherWorker->dispatchEvents();
    }

    g_count++;
    //if (g_count < 100)
    {
        //std::cout << count << '\n';
        if (m_loaded) {
            script::CommandEngine::get().update(ctx);
        }

        {
            KI_TIMER("registry");
            m_registry->updateWT(ctx);
        }

        if (m_loaded) {
            {
                KI_TIMER("node1   ");
                nodeRegistry.updateWT(ctx);
                nodeRegistry.updateModelMatrices();
                nodeRegistry.snapshotWT();
            }
            {
                KI_TIMER("node2   ");
                ControllerRegistry::get().updateWT(ctx);
                nodeRegistry.updateModelMatrices();
            }
            {
                KI_TIMER("audio   ");
                audio::AudioEngine::get().update(ctx);
            }
            {
                KI_TIMER("decal   ");
                decal::DecalSystem::get().updateWT(ctx);
            }
            {
                KI_TIMER("physics0");
                physicsEngine.updatePrepare(ctx);
            }
            {
                KI_TIMER("physics2");
                physicsEngine.updateObjects(ctx);
            }
        }
    }

    // NOTE KI sync to RT
    {
        KI_TIMER("node4   ");
        nodeRegistry.updateModelMatrices();
        nodeRegistry.snapshotWT();
        nodeRegistry.snapshotPending();
    }

    if (m_loaded) {
        physicsEngine.setEnabled(true);
    }
}

void SceneUpdater::handleNodeAdded(Node* node)
{
    if (!node) return;

    animation::AnimationSystem::get().handleNodeAdded(node);
}
