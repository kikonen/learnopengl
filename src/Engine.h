#pragma once

#include "ki/GL.h"

#include "kigl/GLState.h"

#include "asset/Assets.h"
#include "registry/ShaderRegistry.h"

#include "backend/RenderSystem.h"

#include "gui/Window.h"

class Scene;
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

protected:
    virtual int onInit() = 0;
    virtual int onSetup() = 0;
    virtual int onRender(const ki::RenderClock& clock) = 0;
    virtual void onDestroy();

public:
    bool m_debug = false;

    ki::RenderClock m_startClock;

    // NOTE KI MUST destroy async loaded *BEFORE* other registries
    // => alloes change for graceful exit for loaders
    std::shared_ptr<AsyncLoader> m_asyncLoader;
    std::shared_ptr<ShaderRegistry> m_shaderRegistry;

    std::shared_ptr<Scene> m_currentScene;

    std::unique_ptr<Window> m_window;
protected:
    Assets m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    std::string m_title;

    GLState m_state;

    std::unique_ptr<backend::RenderSystem> m_backend;
};
