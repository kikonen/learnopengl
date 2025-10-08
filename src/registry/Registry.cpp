#include "Registry.h"

#include <glm/gtx/quaternion.hpp>

#include "material/MaterialSSBO.h"

#include "util/thread.h"

#include "event/Dispatcher.h"

#include "audio/AudioSystem.h"

#include "debug/DebugContext.h"

#include "engine/PrepareContext.h"

#include "physics/PhysicsSystem.h"

#include "shader/ProgramRegistry.h"

#include "script/CommandEngine.h"
#include "script/ScriptSystem.h"

#include "particle/ParticleSystem.h"
#include "decal/DecalSystem.h"
#include "decal/DecalRegistry.h"

#include "animation/AnimationSystem.h"

#include "terrain/TerrainTileRegistry.h"

#include "text/FontRegistry.h"
#include "text/TextSystem.h"

#include "material/MaterialRegistry.h"
#include "material/ImageRegistry.h"

#include "nav/NavigationSystem.h"

#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/NodeTypeRegistry.h"
#include "registry/SelectionRegistry.h"
#include "registry/VaoRegistry.h"


Registry::Registry(
    Engine& engine,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_engine{ engine },
    m_alive(alive),
    // registries
    m_dispatcherWorkerImpl(std::make_unique<event::Dispatcher>()),
    m_dispatcherViewImpl(std::make_unique<event::Dispatcher>()),
    // pointers
    m_dispatcherWorker(m_dispatcherWorkerImpl.get()),
    m_dispatcherView(m_dispatcherViewImpl.get()),
    m_nodeRegistry{ &NodeRegistry::get() },
    m_selectionRegistry{ &SelectionRegistry::get() }
{
}

Registry::~Registry()
{
}

void Registry::clear()
{
    ASSERT_RT();

    debug::DebugContext::modify().clear();

    //FileEntryCache::get().clear();

    NodeRegistry::get().clear();
    NodeTypeRegistry::get().clear();

    ImageRegistry::get().clear();
    MaterialRegistry::get().clear();
    //ProgramRegistry::get().clear();

    audio::AudioSystem::get().clear();
    physics::PhysicsSystem::get().clear();

    nav::NavigationSystem::get().clear();

    particle::ParticleSystem::get().clear();

    decal::DecalRegistry::get().clear();
    decal::DecalSystem::get().clear();

    animation::AnimationSystem::get().clear();
    terrain::TerrainTileRegistry::get().clear();

    text::TextSystem::get().clear();
    //text::FontRegistry::get().clear();

    script::CommandEngine::get().clear();
    script::ScriptSystem::get().clear();

    ControllerRegistry::get().clear();
    ModelRegistry::get().clear();
    EntityRegistry::get().clear();
    ViewportRegistry::get().clear();
    SelectionRegistry::get().clear();
    VaoRegistry::get().clear();

    physics::PhysicsSystem::get().clear();
    audio::AudioSystem::get().clear();

    ControllerRegistry::get().clear();

    script::CommandEngine::get().clear();
    script::ScriptSystem::get().clear();

    animation::AnimationSystem::get().clear();

    decal::DecalSystem::get().clear();

    text::TextSystem::get().clear();

    VaoRegistry::get().clear();

    ViewportRegistry::get().clear();
}

void Registry::prepare(const PrepareContext& ctx)
{
    ASSERT_RT();

    if (m_prepared) return;
    m_prepared = true;

    m_dispatcherWorker->prepare();
    m_dispatcherWorker->prepare();
    m_dispatcherView->prepare();

    MaterialRegistry::get().prepare();
    EntityRegistry::get().prepare();
    ModelRegistry::get().prepare(m_alive);

    NodeRegistry::get().prepare(&m_engine);
    SelectionRegistry::get().prepare(this);

    physics::PhysicsSystem::get().prepare(m_alive);
    particle::ParticleSystem::get().prepare();
    decal::DecalSystem::get().prepare();
    audio::AudioSystem::get().prepare();
    animation::AnimationSystem::get().prepare();

    terrain::TerrainTileRegistry::get().prepare();

    script::CommandEngine::get().prepare(this);
    script::ScriptSystem::get().prepare(ctx, &script::CommandEngine::get());

    text::TextSystem::get().prepare();
    VaoRegistry::get().prepare();
    ViewportRegistry::get().prepare();
    ControllerRegistry::get().prepare(&m_engine);
}

void Registry::updateWT(const UpdateContext& ctx)
{
    ASSERT_WT();
    //ControllerRegistry::get().updateWT(ctx);
    terrain::TerrainTileRegistry::get().updateWT(ctx);
}

void Registry::updateRT(const UpdateContext& ctx)
{
    ASSERT_RT();

    ProgramRegistry::get().dirtyCheck(ctx);
    ProgramRegistry::get().updateRT(ctx);

    MaterialRegistry::get().updateRT(ctx);
    NodeRegistry::get().updateRT(ctx);
    //SelectionRegistry::get().updateRT(ctx);
    EntityRegistry::get().updateRT(ctx);
    particle::ParticleSystem::get().updateRT(ctx);
    decal::DecalSystem::get().updateRT(ctx);
    animation::AnimationSystem::get().updateRT(ctx);
    terrain::TerrainTileRegistry::get().updateRT(ctx);

    text::FontRegistry::get().updateRT(ctx);
    text::TextSystem::get().updateRT(ctx);

    VaoRegistry::get().updateRT(ctx);
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
