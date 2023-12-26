#pragma once

#include <atomic>

#include "asset/Assets.h"

namespace event
{
    class Dispatcher;
}

namespace audio
{
    class AudioEngine;
}

namespace script
{
    class CommandEngine;
    class ScriptEngine;
}

namespace physics
{
    class PhysicsEngine;
}

class ProgramRegistry;
class MaterialRegistry;
class SpriteRegistry;
class MeshTypeRegistry;
class ModelRegistry;
class NodeRegistry;
class EntityRegistry;
class ViewportRegistry;
class ControllerRegistry;
class ProgramRegistry;

class UpdateContext;
class UpdateViewContext;

//
// Container for all registries to simplify passing them around
//
class Registry {
public:
    Registry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~Registry();

    void prepare();

    void update(const UpdateContext& ctx);
    void updateView(const UpdateViewContext& ctx);

private:
    const Assets& m_assets;

    bool m_prepared = false;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::unique_ptr<event::Dispatcher> m_dispatcherImpl;
    std::unique_ptr<event::Dispatcher> m_dispatcherViewImpl;

    std::unique_ptr<ProgramRegistry> m_programRegistryImpl;

    std::unique_ptr<audio::AudioEngine> m_audioEngineImpl;
    std::unique_ptr<physics::PhysicsEngine> m_physicsEngineImpl;

    std::unique_ptr<script::CommandEngine> m_commandEngineImpl;
    std::unique_ptr<script::ScriptEngine> m_scriptEngineImpl;

    std::unique_ptr<MaterialRegistry> m_materialRegistryImpl;
    std::unique_ptr<SpriteRegistry> m_spriteRegistryImpl;
    std::unique_ptr<MeshTypeRegistry> m_typeRegistryImpl;
    std::unique_ptr<ModelRegistry> m_modelRegistryImpl;
    std::unique_ptr<NodeRegistry> m_nodeRegistryImpl;
    std::unique_ptr<EntityRegistry> m_entityRegistryImpl;
    std::unique_ptr<ViewportRegistry> m_viewportRegistryImpl;
    std::unique_ptr<ControllerRegistry> m_controllerRegistryImpl;

public:
    // NOTE KI initialization order!
    event::Dispatcher* const m_dispatcher;
    event::Dispatcher* const m_dispatcherView;

    ProgramRegistry* const m_programRegistry;

    audio::AudioEngine* const m_audioEngine;
    physics::PhysicsEngine* const m_physicsEngine;

    script::CommandEngine* const m_commandEngine;
    script::ScriptEngine* const m_scriptEngine;

    MaterialRegistry* const m_materialRegistry;
    SpriteRegistry* const m_spriteRegistry;
    MeshTypeRegistry* const m_typeRegistry;
    ModelRegistry* const m_modelRegistry;
    NodeRegistry* const m_nodeRegistry;
    EntityRegistry* const m_entityRegistry;
    ViewportRegistry* const m_viewportRegistry;
    ControllerRegistry* const m_controllerRegistry;
};
