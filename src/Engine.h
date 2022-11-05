#pragma once

#include "ki/GL.h"

#include "asset/GLState.h"

#include "asset/Assets.h"
#include "asset/ShaderRegistry.h"

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
    virtual int onRender(const RenderClock& clock) = 0;
    virtual void onDestroy();

public:
    bool debug = false;

    RenderClock startClock;

    // NOTE KI MUST destroy async loaded *BEFORE* other registries
    // => alloes change for graceful exit for loaders
    std::shared_ptr<AsyncLoader> asyncLoader;
    ShaderRegistry shaders;

    std::shared_ptr<Scene> currentScene;

    std::unique_ptr<Window> window;
protected:
    Assets assets;

    std::string title;

    GLState state;

    std::unique_ptr<backend::RenderSystem> m_backend;
};
