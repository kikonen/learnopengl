#include "SceneUpdater.h"

#include "model/Node.h"

#include "pool/NodeHandle.h"

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
    : Updater("WT", 5, registry, alive)
{}

void SceneUpdater::prepare()
{
    if (m_prepared) return;
    Updater::prepare();

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
            auto handle = pool::NodeHandle::toHandle(e.body.physics.target);

            auto id = pe.registerObject();

            if (id) {
                auto* obj = pe.getObject(id);
                obj->m_update = data.update;
                obj->m_body = data.body;
                obj->m_geom = data.geom;
                obj->m_nodeHandle = handle;
            }
        });

    m_prepared = true;
}

uint32_t SceneUpdater::getActiveCount() const noexcept
{
    return m_registry->m_nodeRegistry->getNodeCount();
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
                KI_TIMER("physics ");
                physics::PhysicsEngine::get().update(ctx);
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

    physics::PhysicsEngine::get().handleNodeAdded(node);
}
