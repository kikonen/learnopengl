#pragma once

#include "kigl/kigl.h"

#include "ki/RenderClock.h"

#include "registry/Registry.h"

#include "ki/FpsCounter.h"
#include "ki/FpsCounter.h"

#include "gui/Input.h"

#include "debug/DebugContext.h"

class Window;
class Scene;
class SceneUpdater;
class ParticleUpdater;
class AnimationUpdater;
class AsyncLoader;

/**
 * Base engine
 */
class Engine {
public:
    Engine();
    virtual ~Engine();

    int init();
    int setup();
    void run();

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

protected:
    virtual int onInit() = 0;
    virtual int onSetup() = 0;

    virtual int onUpdate(const ki::RenderClock& clock) = 0;
    virtual int onRender(const ki::RenderClock& clock) = 0;
    virtual int onPost(const ki::RenderClock& clock) = 0;

    virtual void onDestroy();

    virtual void showFps(const ki::FpsCounter& fpsCounter);

public:
    bool m_debug = false;

    ki::RenderClock m_startClock;

    // NOTE KI MUST destroy async loaded *BEFORE* other registries
    // => alloes change for graceful exit for loaders
    std::shared_ptr<AsyncLoader> m_asyncLoader;

    std::shared_ptr<Registry> m_registry;

    std::shared_ptr<Scene> m_currentScene;
    std::shared_ptr<SceneUpdater> m_sceneUpdater;
    std::shared_ptr<ParticleUpdater> m_particleUpdater;
    std::shared_ptr<AnimationUpdater> m_animationUpdater;

    std::shared_ptr<Window> m_window;

    debug::DebugContext& m_dbg;

protected:
    ki::RenderClock m_clock;
    ki::FpsCounter m_fpsCounter;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::string m_title;

    InputState m_lastInputState;
};
