#include "Registry.h"

#include <glm/gtx/quaternion.hpp>

#include "asset/MaterialSSBO.h"


Registry::Registry(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_alive(alive),
    // registries
    m_programRegistryImpl(assets, m_alive),
    m_materialRegistryImpl(assets, m_alive),
    m_spriteRegistryImpl(assets, m_alive),
    m_typeRegistryImpl(assets, m_alive),
    m_modelRegistryImpl(assets, m_alive),
    m_nodeRegistryImpl(assets, m_alive),
    m_entityRegistryImpl(assets),
    m_viewportRegistryImpl(assets),
    m_controllerRegistryImpl(assets),
    m_physicsEngineImpl(assets),
    m_audioEngineImpl(assets),
    m_dispatcherImpl(assets),
    // pointers
    m_programRegistry(&m_programRegistryImpl),
    m_materialRegistry(&m_materialRegistryImpl),
    m_spriteRegistry(&m_spriteRegistryImpl),
    m_typeRegistry(&m_typeRegistryImpl),
    m_modelRegistry(&m_modelRegistryImpl),
    m_nodeRegistry(&m_nodeRegistryImpl),
    m_entityRegistry(&m_entityRegistryImpl),
    m_viewportRegistry(&m_viewportRegistryImpl),
    m_controllerRegistry(&m_controllerRegistryImpl),
    m_physicsEngine(&m_physicsEngineImpl),
    m_audioEngine(&m_audioEngineImpl),
    m_dispatcher(&m_dispatcherImpl)
{
}

void Registry::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    m_dispatcherImpl.prepare();

    m_materialRegistryImpl.prepare();
    m_spriteRegistryImpl.prepare();
    m_entityRegistryImpl.prepare();
    m_modelRegistryImpl.prepare();

    m_viewportRegistryImpl.prepare();
    m_controllerRegistryImpl.prepare(this);

    m_nodeRegistryImpl.prepare(this);

    m_physicsEngineImpl.prepare();
    m_audioEngineImpl.prepare();
}

void Registry::update(const UpdateContext& ctx)
{
    m_controllerRegistryImpl.update(ctx);
    m_materialRegistryImpl.update(ctx);
    m_spriteRegistryImpl.update(ctx);
    m_modelRegistryImpl.update(ctx);
    m_entityRegistryImpl.update(ctx);
}
