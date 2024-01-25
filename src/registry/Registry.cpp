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

#include "registry/ProgramRegistry.h"

#include "registry/FontRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/SpriteRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/SnapshotRegistry.h"

Registry::Registry(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_alive(alive),
    // registries
    m_dispatcherImpl(std::make_unique<event::Dispatcher>()),
    m_dispatcherViewImpl(std::make_unique<event::Dispatcher>()),
    m_programRegistryImpl(std::make_unique<ProgramRegistry>(assets, m_alive)),
    m_audioEngineImpl(std::make_unique<audio::AudioEngine>()),
    m_physicsEngineImpl(std::make_unique<physics::PhysicsEngine>(assets, m_alive)),
    m_commandEngineImpl(std::make_unique<script::CommandEngine>()),
    m_scriptEngineImpl(std::make_unique<script::ScriptEngine>()),
    m_fontRegistryImpl(std::make_unique<FontRegistry>(assets)),
    m_materialRegistryImpl(std::make_unique<MaterialRegistry>(assets, m_alive)),
    m_spriteRegistryImpl(std::make_unique<SpriteRegistry>(assets, m_alive)),
    m_typeRegistryImpl(std::make_unique<MeshTypeRegistry>(assets)),
    m_modelRegistryImpl(std::make_unique<ModelRegistry>(assets, m_alive)),
    m_nodeRegistryImpl(std::make_unique<NodeRegistry>(assets, m_alive)),
    m_snapshotRegistryImpl{std::make_unique<SnapshotRegistry>()},
    m_entityRegistryImpl(std::make_unique<EntityRegistry>(assets)),
    m_viewportRegistryImpl(std::make_unique<ViewportRegistry>(assets)),
    m_controllerRegistryImpl(std::make_unique<ControllerRegistry>(assets)),
    // pointers
    m_dispatcher(m_dispatcherImpl.get()),
    m_dispatcherView(m_dispatcherViewImpl.get()),
    m_programRegistry(m_programRegistryImpl.get()),
    m_audioEngine(m_audioEngineImpl.get()),
    m_physicsEngine(m_physicsEngineImpl.get()),
    m_commandEngine(m_commandEngineImpl.get()),
    m_scriptEngine(m_scriptEngineImpl.get()),
    m_fontRegistry(m_fontRegistryImpl.get()),
    m_materialRegistry(m_materialRegistryImpl.get()),
    m_spriteRegistry(m_spriteRegistryImpl.get()),
    m_typeRegistry(m_typeRegistryImpl.get()),
    m_modelRegistry(m_modelRegistryImpl.get()),
    m_nodeRegistry(m_nodeRegistryImpl.get()),
    m_snapshotRegistry{ m_snapshotRegistryImpl.get() },
    m_entityRegistry(m_entityRegistryImpl.get()),
    m_viewportRegistry(m_viewportRegistryImpl.get()),
    m_controllerRegistry(m_controllerRegistryImpl.get())
{
}

Registry::~Registry()
{
}

void Registry::prepareShared()
{
    if (m_prepared) return;
    m_prepared = true;

    m_dispatcher->prepare();
    m_dispatcherView->prepare();

    m_materialRegistry->prepare();
    m_spriteRegistry->prepare();
    m_entityRegistry->prepare();
    m_modelRegistry->prepare();

    m_viewportRegistry->prepare();

    m_nodeRegistry->prepare(this);
}

void Registry::prepareWT()
{
    ASSERT_WT();

    PrepareContext ctx{ m_assets, this };

    // NOTE KI does not matter which thread does prepare
    m_physicsEngine->prepare();
    m_audioEngine->prepare();

    m_controllerRegistry->prepare(this);

    m_commandEngine->prepare(this);
    m_scriptEngine->prepare(ctx, m_commandEngine);
}

void Registry::updateWT(const UpdateContext& ctx)
{
    ASSERT_WT();
    //m_controllerRegistry->updateWT(ctx);
}

void Registry::updateRT(const UpdateContext& ctx)
{
    ASSERT_RT();
    m_fontRegistry->updateRT(ctx);
    m_materialRegistry->updateRT(ctx);
    m_spriteRegistry->updateRT(ctx);
    m_modelRegistry->updateRT(ctx);
    m_nodeRegistry->updateRT(ctx);
    m_entityRegistry->updateRT(ctx);
}

void Registry::postRT(const UpdateContext& ctx)
{
    ASSERT_RT();
    m_entityRegistry->postRT(ctx);
}

void Registry::withLock(const std::function<void(Registry&)>& fn)
{
    std::lock_guard<std::mutex> lock(m_lock);
    fn(*this);
}
