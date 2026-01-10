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
#include "engine/PrepareContext.h"

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
    Engine& engine)
    : Updater("WT", 5, engine)
{}

void SceneUpdater::shutdown()
{
    if (!m_prepared) return;
    {
        script::CommandEngine::get().clear();
        script::ScriptSystem::get().stop();
    }
    Updater::shutdown();
}

void SceneUpdater::prepare()
{
    if (m_prepared) return;
    Updater::prepare();

    const auto& assets = Assets::get();
    auto* registry = getRegistry();

    std::lock_guard lock(m_prepareLock);

    auto* dispatcher = registry->m_dispatcherWorker;

    {
        auto& scriptSystem = script::ScriptSystem::get();
        scriptSystem.start();
    }

    m_listen_scene_loaded.listen(
        event::Type::scene_loaded,
        dispatcher,
        [this](const event::Event& e) {
            m_loaded = true;

            // NOTE KI trigger UI sidew update *after* all worker side processing done
            {
                event::Event evt { event::Type::scene_loaded };
                getRegistry()->m_dispatcherView->send(evt);
            }
        });

    m_listen_scene_unload.listen(
        event::Type::scene_loaded,
        dispatcher,
        [this](const event::Event& e) {
            m_loaded = true;

            // NOTE KI trigger UI sidew update *after* all worker side processing done
            {
                event::Event evt{ event::Type::scene_loaded };
                getRegistry()->m_dispatcherView->send(evt);
            }
        });

    m_listen_node_added.listen(
        event::Type::node_added,
        dispatcher,
        [this](const event::Event& e) {
            auto* node = pool::NodeHandle::toNode(e.body.node.target);
            this->handleNodeAdded(node);
        });

    m_listen_node_removed.listen(
        event::Type::node_removed,
        dispatcher,
        [this](const event::Event& e) {
            auto* node = pool::NodeHandle::toNode(e.body.node.target);
            this->handleNodeRemoved(node);
        });

    if (assets.useScript)
    {
        m_listen_script_run.listen(
            event::Type::script_run,
            dispatcher,
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

    auto* registry = getRegistry();
    auto& nodeRegistry = NodeRegistry::get();
    auto& physicsSystem = physics::PhysicsSystem::get();
    auto& scriptSystem = script::ScriptSystem::get();

    {
        KI_TIMER("event   ");
        registry->m_dispatcherWorker->dispatchEvents();
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
            registry->updateWT(ctx);
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
        nodeRegistry.publishSnapshots();
        nodeRegistry.notifyPendingChanges();
    }

    if (physicsSystem.isEnabled()) {
        nav::NavigationSystem::get().build();
    }
    if (m_loaded) {
        physicsSystem.setEnabled(true);
    }
}

void SceneUpdater::handleNodeAdded(model::Node* node)
{
    if (!node) return;

    animation::AnimationSystem::get().handleNodeAdded(node);
    if (node->m_typeFlags.navMesh) {
        nav::NavigationSystem::get().registerNode(node->toHandle());
    }
}

void SceneUpdater::handleNodeRemoved(model::Node* node)
{
    if (!node) return;

    animation::AnimationSystem::get().handleNodeRemoved(node);
    if (node->m_typeFlags.navMesh) {
        nav::NavigationSystem::get().unregisterNode(node->toHandle());
    }
}

std::string SceneUpdater::getStats()
{
    auto* registry = getRegistry();
    auto& commandEngine = script::CommandEngine::get();
    const auto pendingCommandCount = commandEngine.getPendingCount();
    const auto blockedCommandCount = commandEngine.getBlockedCount();
    const auto activeCommandCount = commandEngine.getActiveCount();

    const auto nodeCount = registry->m_nodeRegistry->getNodeCount();
    const auto decalCount = decal::DecalSystem::get().getActiveDecalCount();
    const auto physicsCount = physics::PhysicsSystem::get().getObjectCount();

    return fmt::format(
        "nodes={}, decals={}, physics={}, cmd_pending={}, cmd_blocked={}, cmd_active={}",
        nodeCount, decalCount, physicsCount,
        pendingCommandCount, blockedCommandCount, activeCommandCount);
}
