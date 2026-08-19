#pragma once

#include <glm/glm.hpp>

#include "kigl/kigl.h"

#include "ki/RenderClock.h"

#include "registry/Registry.h"

#include "ki/FpsCounter.h"

#include "util/Ref.h"

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

struct UpdateContext;
struct UpdateViewContext;

namespace render
{
    class Batch;
    class RenderData;
    class WindowBuffer;
}

/**
 * Base engine
 */
class Engine : public util::RefCounted<> {
public:
    Engine();
    virtual ~Engine();

    bool init();
    void run();

    bool renderFrame();

    inline std::shared_ptr<std::atomic_bool>& getAlive() const noexcept
    {
        return m_alive;
    }

    inline const util::Ref<Registry>& getRegistry() const noexcept {
        return m_registry;
    }

    inline const ki::RenderClock& getClock() const noexcept {
        return m_clock;
    }

    inline const ki::FpsCounter& getFpsCounter() const noexcept {
        return m_fpsCounter;
    }

    const util::Ref<Scene>& getCurrentScene() const;

    render::Batch* getBatch() const noexcept
    {
        return m_batch.get();
    }

    render::RenderData* getRenderData() const noexcept
    {
        return m_renderData.get();
    }

    util::Ref<Window> getWindow() const;

    render::WindowBuffer* getWindowBuffer() const noexcept
    {
        return m_windowBuffer.get();
    }

    const glm::ivec2& getSize() const;

    backend::gl::PerformanceCounters getCounters(bool clear) const;
    backend::gl::PerformanceCounters getCountersLocal(bool clear) const;

protected:
    bool setup();
    void update();
    void updateView();
    void render();
    void processInput();

protected:
    virtual bool onInit() { return false; };
    virtual bool onSetup() { return false; };

    virtual void onUpdate(const UpdateContext& ctx) {};
    virtual void onUpdateView(const UpdateViewContext& ctx) {};
    virtual void onRender(const ki::RenderClock& clock) {};

    virtual void onDestroy();

    virtual void showFps(const ki::FpsCounter& fpsCounter);

    void prepareUBOs();
    void updateUBOs() const;

    Assets loadAssets();

public:
    bool m_debug = false;

    ki::RenderClock m_startClock;

    util::Ref<Registry> m_registry;

    util::Ref<SceneUpdater> m_sceneUpdater;
    util::Ref<ParticleUpdater> m_particleUpdater;
    util::Ref<AnimationUpdater> m_animationUpdater;

    util::Ref<Window> m_window;
    std::unique_ptr<render::WindowBuffer> m_windowBuffer{ nullptr };

    debug::DebugContext& m_dbg;

protected:
    ki::RenderClock m_clock;
    ki::FpsCounter m_fpsCounter;

    mutable std::shared_ptr<std::atomic_bool> m_alive;

    std::string m_title;

    util::Ref<Scene> m_currentScene;

    std::unique_ptr<render::Batch> m_batch;
    std::unique_ptr<render::RenderData> m_renderData;
    DataUBO m_dataUBO;
    DebugUBO m_debugUBO;
};
