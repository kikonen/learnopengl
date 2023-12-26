#include "Registry.h"

#include <glm/gtx/quaternion.hpp>

#include "asset/MaterialSSBO.h"

#include "event/Dispatcher.h"

#include "audio/AudioEngine.h"

#include "script/api/Command.h"
#include "script/CommandEngine.h"
#include "script/CommandAPI.h"
#include "script/ScriptEngine.h"

#include "physics/PhysicsEngine.h"

#include "registry/ProgramRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/SpriteRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ControllerRegistry.h"


Registry::Registry(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_alive(alive),
    // registries
    m_dispatcherImpl(std::make_unique<event::Dispatcher>(assets)),
    m_dispatcherViewImpl(std::make_unique<event::Dispatcher>(assets)),
    m_programRegistryImpl(std::make_unique<ProgramRegistry>(assets, m_alive)),
    m_audioEngineImpl(std::make_unique<audio::AudioEngine>(assets)),
    m_physicsEngineImpl(std::make_unique<physics::PhysicsEngine>(assets)),
    m_commandEngineImpl(std::make_unique<script::CommandEngine>(assets)),
    m_scriptEngineImpl(std::make_unique<script::ScriptEngine>(assets)),
    m_materialRegistryImpl(std::make_unique<MaterialRegistry>(assets, m_alive)),
    m_spriteRegistryImpl(std::make_unique<SpriteRegistry>(assets, m_alive)),
    m_typeRegistryImpl(std::make_unique<MeshTypeRegistry>(assets, m_alive)),
    m_modelRegistryImpl(std::make_unique<ModelRegistry>(assets, m_alive)),
    m_nodeRegistryImpl(std::make_unique<NodeRegistry>(assets, m_alive)),
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
    m_materialRegistry(m_materialRegistryImpl.get()),
    m_spriteRegistry(m_spriteRegistryImpl.get()),
    m_typeRegistry(m_typeRegistryImpl.get()),
    m_modelRegistry(m_modelRegistryImpl.get()),
    m_nodeRegistry(m_nodeRegistryImpl.get()),
    m_entityRegistry(m_entityRegistryImpl.get()),
    m_viewportRegistry(m_viewportRegistryImpl.get()),
    m_controllerRegistry(m_controllerRegistryImpl.get())
{
}

Registry::~Registry()
{
}

void Registry::prepare()
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
    m_controllerRegistry->prepare(this);

    m_nodeRegistry->prepare(this);
}

void Registry::prepareWorker()
{
    // NOTE KI does not matter which thread does prepare
    m_physicsEngine->prepare();
    m_audioEngine->prepare();

    m_commandEngine->prepare(this);
    m_scriptEngine->prepare(m_commandEngine);
}

void Registry::update(const UpdateContext& ctx)
{
    m_controllerRegistry->update(ctx);
    m_entityRegistry->update(ctx);
}

void Registry::updateView(const UpdateViewContext& ctx)
{
    m_materialRegistry->updateView(ctx);
    m_spriteRegistry->updateView(ctx);
    m_modelRegistry->updateView(ctx);
    m_entityRegistry->updateView(ctx);
}
