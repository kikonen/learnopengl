#include "SceneUpdater.h"

#include <fmt/format.h>

#include "ki/Timer.h"

#include "asset/Assets.h"

#include "model/Node.h"

#include "pool/NodeHandle.h"

#include "util/Log.h"

#include "event/Dispatcher.h"

#include "script/CommandEngine.h"

#include "animation/AnimationSystem.h"
#include "decal/DecalSystem.h"

#include "engine/UpdateContext.h"

#include "physics/PhysicsSystem.h"

#include "nav/NavigationSystem.h"

#include "script/CommandEngine.h"
#include "script/ScriptSystem.h"

#include "registry/Registry.h"
#include "registry/ControllerRegistry.h"
#include "registry/NodeRegistry.h"

#define KI_TIMER(x)

namespace {
    size_t g_count = 0;
}

SceneUpdater::SceneUpdater(
    std::shared_ptr<Registry> registry,
    std::shared_ptr<std::atomic<bool>> alive)
    : Updater("WT", 5, registry, alive)
{}

void SceneUpdater::shutdown()
{
    if (!m_prepared) return;
    Updater::shutdown();

    m_registry->shutdownWT();
}

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
        });

    dispatcher->addListener(
        event::Type::node_removed,
        [this](const event::Event& e) {
            auto* node = pool::NodeHandle::toNode(e.body.node.target);
            this->handleNodeRemoved(node);
        });

    if (assets.useScript)
    {
        dispatcher->addListener(
            event::Type::script_run,
            [this](const event::Event& e) {
                auto& data = e.body.script;

                const auto* node = pool::NodeHandle::toNode(data.target);

                if (node) {
                    auto& scriptSystem = script::ScriptSystem::get();

                    if (data.global) {
                        scriptSystem.runGlobalScript(node, data.id);
                    }
                    else {
                        scriptSystem.runNodeScript(node, data.id);
                    }
                }
            });
    }

    m_prepared = true;
}

void SceneUpdater::update(const UpdateContext& ctx)
{
    KI_TIMER("[loop]  ");

    auto& nodeRegistry = NodeRegistry::get();
    auto& physicsSystem = physics::PhysicsSystem::get();
    auto& scriptSystem = script::ScriptSystem::get();

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
                KI_TIMER("node2   ");
                ControllerRegistry::get().updateWT(ctx);
            }
            {
                KI_TIMER("node1   ");
                nodeRegistry.updateWT(ctx);
            }
            {
                auto entityIndex = nodeRegistry.validateModelMatrices();
                if (entityIndex != -1) {
                    KI_CRITICAL(fmt::format("UNSTABLE NODE_TREE: index={}", entityIndex));
                    nodeRegistry.updateModelMatrices();
                }
            }
            {
                KI_TIMER("script");
                scriptSystem.update(ctx);
                nodeRegistry.updateModelMatrices();
            }
            {
                KI_TIMER("physics0");
                physicsSystem.updatePrepare(ctx);
            }
            {
                KI_TIMER("physics2");
                physicsSystem.updateObjects(ctx);
            }
            {
                nodeRegistry.updateModelMatrices();
                nodeRegistry.postUpdateWT(ctx);
                decal::DecalSystem::get().updateWT(ctx);
            }
        }
         else {
             nodeRegistry.updateModelMatrices();
        }
    }

    // NOTE KI sync to RT
    {
        KI_TIMER("node4   ");
        nodeRegistry.snapshotPending();
        nodeRegistry.notifyPendingChanges();
    }

    if (physicsSystem.isEnabled()) {
        nav::NavigationSystem::get().build();
    }
    if (m_loaded) {
        physicsSystem.setEnabled(true);
    }
}

void SceneUpdater::handleNodeAdded(Node* node)
{
    if (!node) return;

    animation::AnimationSystem::get().handleNodeAdded(node);
    if (node->m_typeFlags.navMesh) {
        nav::NavigationSystem::get().registerNode(node->toHandle());
    }
}

void SceneUpdater::handleNodeRemoved(Node* node)
{
    if (!node) return;

    animation::AnimationSystem::get().handleNodeRemoved(node);
    if (node->m_typeFlags.navMesh) {
        nav::NavigationSystem::get().unregisterNode(node->toHandle());
    }
}

std::string SceneUpdater::getStats()
{
    auto& commandEngine = script::CommandEngine::get();
    const auto pendingCommandCount = commandEngine.getPendingCount();
    const auto blockedCommandCount = commandEngine.getBlockedCount();
    const auto activeCommandCount = commandEngine.getActiveCount();

    const auto nodeCount = m_registry->m_nodeRegistry->getNodeCount();
    const auto decalCount = decal::DecalSystem::get().getActiveDecalCount();
    const auto physicsCount = physics::PhysicsSystem::get().getObjectCount();

    return fmt::format(
        "nodes={}, decals={}, physics={}, cmd_pending={}, cmd_blocked={}, cmd_active={}",
        nodeCount, decalCount, physicsCount,
        pendingCommandCount, blockedCommandCount, activeCommandCount);
}
