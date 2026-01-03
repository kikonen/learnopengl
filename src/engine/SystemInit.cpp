#include "SystemInit.h"

#include "audio/AudioSystem.h"

#include "physics/PhysicsSystem.h"

#include "shader/FileEntryCache.h"
#include "shader/ProgramRegistry.h"

#include "script/CommandEngine.h"
#include "script/ScriptSystem.h"

#include "particle/ParticleSystem.h"

#include "decal/DecalRegistry.h"
#include "decal/DecalSystem.h"

#include "animation/AnimationSystem.h"

#include "terrain/TerrainTileRegistry.h"

#include "text/FontRegistry.h"
#include "text/TextSystem.h"

#include "material/MaterialRegistry.h"
#include "material/ImageRegistry.h"

#include "nav/NavigationSystem.h"

#include "render/InstanceRegistry.h"

#include "registry/NodeTypeRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/MeshSetRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/SelectionRegistry.h"
#include "registry/VaoRegistry.h"

void SystemInit::init() noexcept
{
    FileEntryCache::init();

    ImageRegistry::init();
    ProgramRegistry::init();
    MeshSetRegistry::init();
    text::FontRegistry::init();

    MaterialRegistry::init();

    audio::AudioSystem::init();
    physics::PhysicsSystem::init();

    nav::NavigationSystem::init();

    render::InstanceRegistry::init();

    particle::ParticleSystem::init();

    decal::DecalRegistry::init();
    decal::DecalSystem::init();

    animation::AnimationSystem::init();
    terrain::TerrainTileRegistry::init();

    text::TextSystem::init();

    script::CommandEngine::init();
    script::ScriptSystem::init();

    ControllerRegistry::init();
    NodeRegistry::init();
    NodeTypeRegistry::init();
    EntityRegistry::init();
    ViewportRegistry::init();
    SelectionRegistry::init();
    VaoRegistry::init();
}

void SystemInit::release() noexcept
{
    nav::NavigationSystem::release();

    particle::ParticleSystem::release();

    decal::DecalRegistry::release();
    decal::DecalSystem::release();

    animation::AnimationSystem::release();
    terrain::TerrainTileRegistry::release();

    text::TextSystem::release();
    text::FontRegistry::release();

    script::CommandEngine::release();
    script::ScriptSystem::release();

    ProgramRegistry::release();
    ControllerRegistry::release();
    NodeRegistry::release();
    NodeTypeRegistry::release();
    MeshSetRegistry::release();
    EntityRegistry::release();
    ViewportRegistry::release();
    ControllerRegistry::release();
    FileEntryCache::release();

    audio::AudioSystem::release();
    physics::PhysicsSystem::release();

    SelectionRegistry::release();
    VaoRegistry::release();
    MaterialRegistry::release();
    ImageRegistry::release();
}
