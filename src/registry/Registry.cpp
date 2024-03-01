#include "Registry.h"

#include <glm/gtx/quaternion.hpp>

#include "asset/MaterialSSBO.h"

#include "util/thread.h"

#include "event/Dispatcher.h"

#include "audio/AudioEngine.h"

#include "engine/PrepareContext.h"

#include "physics/PhysicsEngine.h"
#include "physics/ObjectSnapshotRegistry.h"

#include "script/api/Command.h"
#include "script/CommandEngine.h"
#include "script/CommandAPI.h"
#include "script/ScriptEngine.h"

#include "particle/ParticleSystem.h"

#include "registry/FontRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/SpriteRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/NodeSnapshotRegistry.h"
#include "registry/NodeRegistry.h"

Registry::Registry(
    std::shared_ptr<std::atomic<bool>> alive)
    : m_alive(alive),
    // registries
    m_dispatcherWorkerImpl(std::make_unique<event::Dispatcher>()),
    m_dispatcherViewImpl(std::make_unique<event::Dispatcher>()),
    m_workerSnapshotRegistryImpl{std::make_unique<NodeSnapshotRegistry>()},
    m_pendingSnapshotRegistryImpl{ std::make_unique<NodeSnapshotRegistry>() },
    m_activeSnapshotRegistryImpl{ std::make_unique<NodeSnapshotRegistry>() },
    m_workerObjectSnapshotRegistryImpl{ std::make_unique<physics::ObjectSnapshotRegistry>() },
    m_pendingObjectSnapshotRegistryImpl{ std::make_unique<physics::ObjectSnapshotRegistry>() },
    m_activeObjectSnapshotRegistryImpl{ std::make_unique<physics::ObjectSnapshotRegistry>() },
    // pointers
    m_dispatcherWorker(m_dispatcherWorkerImpl.get()),
    m_dispatcherView(m_dispatcherViewImpl.get()),
    m_workerSnapshotRegistry{ m_workerSnapshotRegistryImpl.get() },
    m_pendingSnapshotRegistry{ m_pendingSnapshotRegistryImpl.get() },
    m_activeSnapshotRegistry{ m_activeSnapshotRegistryImpl.get() },
    m_workerObjectSnapshotRegistry{ m_workerObjectSnapshotRegistryImpl.get() },
    m_pendingObjectSnapshotRegistry{ m_pendingObjectSnapshotRegistryImpl.get() },
    m_activeObjectSnapshotRegistry{ m_activeObjectSnapshotRegistryImpl.get() },
    m_nodeRegistry{ &NodeRegistry::get() }
{
}

Registry::~Registry()
{
}

void Registry::prepareShared()
{
    if (m_prepared) return;
    m_prepared = true;

    m_dispatcherWorker->prepare();
    m_dispatcherView->prepare();

    MaterialRegistry::get().prepare();
    SpriteRegistry::get().prepare();
    EntityRegistry::get().prepare();
    ModelRegistry::get().prepare(m_alive);

    ViewportRegistry::get().prepare();

    NodeRegistry::get().prepare(this);

    physics::PhysicsEngine::get().prepare(m_alive, this);

    particle::ParticleSystem::get().prepare();
}

void Registry::prepareWT()
{
    ASSERT_WT();

    PrepareContext ctx{ this };

    audio::AudioEngine::get().prepare();

    ControllerRegistry::get().prepare(this);

    script::CommandEngine::get().prepare(this);
    script::ScriptEngine::get().prepare(ctx, &script::CommandEngine::get());

}

void Registry::updateWT(const UpdateContext& ctx)
{
    ASSERT_WT();
    //ControllerRegistry::get().updateWT(ctx);
}

void Registry::updateRT(const UpdateContext& ctx)
{
    ASSERT_RT();
    FontRegistry::get().updateRT(ctx);
    MaterialRegistry::get().updateRT(ctx);
    SpriteRegistry::get().updateRT(ctx);
    ModelRegistry::get().updateRT(ctx);
    NodeRegistry::get().updateRT(ctx);
    EntityRegistry::get().updateRT(ctx);
    particle::ParticleSystem::get().updateRT(ctx);
}

void Registry::postRT(const UpdateContext& ctx)
{
    ASSERT_RT();
    EntityRegistry::get().postRT(ctx);
}

void Registry::withLock(const std::function<void(Registry&)>& fn)
{
    std::lock_guard lock(m_lock);
    fn(*this);
}
