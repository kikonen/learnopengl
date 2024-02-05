#include "Registry.h"

#include <glm/gtx/quaternion.hpp>

#include "asset/MaterialSSBO.h"

#include "util/thread.h"

#include "event/Dispatcher.h"

#include "audio/AudioEngine.h"

#include "engine/PrepareContext.h"

#include "script/api/Command.h"
#include "script/CommandEngine.h"
#include "script/CommandAPI.h"
#include "script/ScriptEngine.h"

#include "physics/PhysicsEngine.h"

#include "registry/FontRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/SpriteRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/SnapshotRegistry.h"


Registry& Registry::get() noexcept
{
    static Registry s_registry;
    return s_registry;
}

Registry::Registry()
    : // registries
    m_dispatcherImpl(std::make_unique<event::Dispatcher>()),
    m_dispatcherViewImpl(std::make_unique<event::Dispatcher>()),
    m_snapshotRegistryImpl{std::make_unique<SnapshotRegistry>()},
    // pointers
    m_dispatcher(m_dispatcherImpl.get()),
    m_dispatcherView(m_dispatcherViewImpl.get()),
    m_snapshotRegistry{ m_snapshotRegistryImpl.get() }
{
}

Registry::~Registry()
{
}

void Registry::prepareShared(std::shared_ptr<std::atomic<bool>> alive)
{
    if (m_prepared) return;
    m_prepared = true;

    m_alive = alive;

    m_dispatcher->prepare();
    m_dispatcherView->prepare();

    MaterialRegistry::get().prepare();
    SpriteRegistry::get().prepare();
    EntityRegistry::get().prepare();
    ModelRegistry::get().prepare(m_alive);

    ViewportRegistry::get().prepare();

    NodeRegistry::get().prepare();
}

void Registry::prepareWT()
{
    ASSERT_WT();

    PrepareContext ctx{};

    // NOTE KI does not matter which thread does prepare
    physics::PhysicsEngine::get().prepare(m_alive);

    audio::AudioEngine::get().prepare();

    ControllerRegistry::get().prepare();

    script::CommandEngine::get().prepare();
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
