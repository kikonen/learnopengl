#include "Registry.h"

#include <glm/gtx/quaternion.hpp>

#include "material/MaterialSSBO.h"

#include "util/thread.h"

#include "event/Dispatcher.h"

#include "audio/AudioSystem.h"

#include "engine/PrepareContext.h"

#include "physics/PhysicsSystem.h"

#include "shader/ProgramRegistry.h"

#include "script/CommandEngine.h"
#include "script/ScriptSystem.h"

#include "particle/ParticleSystem.h"
#include "decal/DecalSystem.h"
#include "animation/AnimationSystem.h"

#include "terrain/TerrainTileRegistry.h"

#include "text/FontRegistry.h"
#include "text/TextSystem.h"

#include "material/MaterialRegistry.h"

#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/SelectionRegistry.h"
#include "registry/VaoRegistry.h"


Registry::Registry(
    std::shared_ptr<std::atomic<bool>> alive)
    : m_alive(alive),
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
{}

void Registry::clearShared()
{
    ASSERT_RT();
}

void Registry::shutdownShared()
{
    ASSERT_RT();
    if (!m_prepared) return;

    clearShared();
}

void Registry::prepareShared()
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

    NodeRegistry::get().prepare(this);
    SelectionRegistry::get().prepare(this);

    physics::PhysicsSystem::get().prepare(m_alive);

    particle::ParticleSystem::get().prepare();

    terrain::TerrainTileRegistry::get().prepare();

    clearShared();
}

void Registry::clearWT()
{
    ASSERT_WT();

    audio::AudioSystem::get().clear();

    ControllerRegistry::get().clear();

    script::CommandEngine::get().clear();
    script::ScriptSystem::get().clear();

    animation::AnimationSystem::get().clearWT();

    decal::DecalSystem::get().clearWT();
}

void Registry::shutdownWT()
{
    ASSERT_WT();

    physics::PhysicsSystem::get().shutdown();

    audio::AudioSystem::get().shutdown();

    ControllerRegistry::get().shutdown();

    script::CommandEngine::get().shutdown();
    script::ScriptSystem::get().shutdown();

    animation::AnimationSystem::get().shutdownWT();

    decal::DecalSystem::get().shutdownWT();
}

void Registry::prepareWT()
{
    ASSERT_WT();

    PrepareContext ctx{ this };

    audio::AudioSystem::get().prepare();

    animation::AnimationSystem::get().prepareWT();

    ControllerRegistry::get().prepare(this);

    script::CommandEngine::get().prepare(this);
    script::ScriptSystem::get().prepare(ctx, &script::CommandEngine::get());

    decal::DecalSystem::get().prepareWT();
}

void Registry::clearRT()
{
    ASSERT_RT();

    animation::AnimationSystem::get().clearRT();

    text::TextSystem::get().clear();

    VaoRegistry::get().clear();

    ViewportRegistry::get().clear();

    decal::DecalSystem::get().clearRT();
}

void Registry::shutdownRT()
{
    ASSERT_RT();

    animation::AnimationSystem::get().shutdownRT();

    text::TextSystem::get().shutdown();

    VaoRegistry::get().shutdown();

    ViewportRegistry::get().shutdown();

    decal::DecalSystem::get().shutdownRT();
}

void Registry::prepareRT()
{
    ASSERT_RT();

    animation::AnimationSystem::get().prepareRT();

    text::TextSystem::get().prepare();

    VaoRegistry::get().prepare();

    ViewportRegistry::get().prepare();

    decal::DecalSystem::get().prepareRT();
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
