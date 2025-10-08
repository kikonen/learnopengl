#pragma once

#include <glm/glm.hpp>

#include "kigl/kigl.h"

#include "ki/RenderClock.h"

#include "registry/Registry.h"

#include "ki/FpsCounter.h"

#include "backend/gl/PerformanceCounters.h"

#include "gui/Input.h"

#include "debug/DebugContext.h"

#include "shader/DataUBO.h"
#include "shader/DebugUBO.h"

class Assets;
class Window;
class Scene;
class SceneUpdater;
class ParticleUpdater;
class AnimationUpdater;
class AsyncLoader;

namespace render
{
    class Batch;
    class RenderData;
    class WindowBuffer;
}

/**
 * Base engine
 */
class Engine {
public:
    Engine();
    virtual ~Engine();

    void run();

    int init();
    int setup();
    int update();
    int render();
    void processInput();

    inline Registry* getRegistry() const noexcept {
        return m_registry.get();
    }

    inline const ki::RenderClock& getClock() const noexcept {
        return m_clock;
    }

    inline const ki::FpsCounter& getFpsCounter() const noexcept {
        return m_fpsCounter;
    }

    std::shared_ptr<Scene> getCurrentScene() const
    {
        return m_currentScene;
    }

    render::Batch* getBatch() const noexcept
    {
        return m_batch.get();
    }

    render::RenderData* getRenderData() const noexcept
    {
        return m_renderData.get();
    }

    std::shared_ptr<Window> getWindow()
    {
        return m_window;
    }

    render::WindowBuffer* getWindowBuffer() const noexcept
    {
        return m_windowBuffer.get();
    }

    const glm::ivec2& getSize() const;

    backend::gl::PerformanceCounters getCounters(bool clear) const;
    backend::gl::PerformanceCounters getCountersLocal(bool clear) const;

protected:
    virtual int onInit() = 0;
    virtual int onSetup() = 0;

    virtual int onUpdate(const UpdateContext& ctx) = 0;
    virtual int onRender(const ki::RenderClock& clock) = 0;
    virtual int onPost(const UpdateContext& ctx) = 0;

    virtual void onDestroy();

    virtual void showFps(const ki::FpsCounter& fpsCounter);

    void prepareUBOs();
    void updateUBOs() const;

    Assets loadAssets();

public:
    bool m_debug = false;

    ki::RenderClock m_startClock;

    // NOTE KI MUST destroy async loaded *BEFORE* other registries
    // => alloes change for graceful exit for loaders
    std::shared_ptr<AsyncLoader> m_asyncLoader;

    std::unique_ptr<Registry> m_registry;

    std::shared_ptr<SceneUpdater> m_sceneUpdater;
    std::shared_ptr<ParticleUpdater> m_particleUpdater;
    std::shared_ptr<AnimationUpdater> m_animationUpdater;

    std::shared_ptr<Window> m_window;
    std::unique_ptr<render::WindowBuffer> m_windowBuffer{ nullptr };

    debug::DebugContext& m_dbg;

protected:
    ki::RenderClock m_clock;
    ki::FpsCounter m_fpsCounter;

    std::shared_ptr<std::atomic_bool> m_alive;

    std::string m_title;

    std::shared_ptr<Scene> m_currentScene;

    std::unique_ptr<render::Batch> m_batch;
    std::unique_ptr<render::RenderData> m_renderData;
    DataUBO m_dataUBO;
    DebugUBO m_debugUBO;
};
