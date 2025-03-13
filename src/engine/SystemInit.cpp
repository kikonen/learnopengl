#include "SystemInit.h"

#include "audio/AudioEngine.h"

#include "physics/PhysicsEngine.h"

#include "shader/ProgramRegistry.h"

#include "script/CommandEngine.h"
#include "script/ScriptEngine.h"

#include "particle/ParticleSystem.h"

#include "decal/DecalRegistry.h"
#include "decal/DecalSystem.h"

#include "animation/AnimationSystem.h"

#include "terrain/TerrainTileRegistry.h"

#include "text/FontRegistry.h"
#include "text/TextSystem.h"

#include "material/MaterialRegistry.h"

#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/ViewportRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/SelectionRegistry.h"
#include "registry/VaoRegistry.h"

void SystemInit::init() noexcept
{
    audio::AudioEngine::init();
    physics::PhysicsEngine::init();

    particle::ParticleSystem::init();

    decal::DecalRegistry::init();
    decal::DecalSystem::init();

    animation::AnimationSystem::init();
    terrain::TerrainTileRegistry::init();

    text::TextSystem::init();
    text::FontRegistry::init();

    script::CommandEngine::init();
    script::ScriptEngine::init();

    ProgramRegistry::init();
    ControllerRegistry::init();
    MaterialRegistry::init();
    NodeRegistry::init();
    MeshTypeRegistry::init();
    ModelRegistry::init();
    EntityRegistry::init();
    ViewportRegistry::init();
    ControllerRegistry::init();
    SelectionRegistry::init();
    VaoRegistry::init();
}

void SystemInit::release() noexcept
{
    audio::AudioEngine::release();
    physics::PhysicsEngine::release();

    particle::ParticleSystem::release();

    decal::DecalRegistry::release();
    decal::DecalSystem::release();

    animation::AnimationSystem::release();
    terrain::TerrainTileRegistry::release();

    text::TextSystem::release();
    text::FontRegistry::release();

    script::CommandEngine::release();
    script::ScriptEngine::release();

    ProgramRegistry::release();
    ControllerRegistry::release();
    MaterialRegistry::release();
    NodeRegistry::release();
    MeshTypeRegistry::release();
    ModelRegistry::release();
    EntityRegistry::release();
    ViewportRegistry::release();
    ControllerRegistry::release();
    SelectionRegistry::release();
    VaoRegistry::release();
}
