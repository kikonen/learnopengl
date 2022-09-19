#pragma once

#include "ki/GL.h"

#include "asset/GLState.h"

#include "asset/Assets.h"
#include "asset/ShaderRegistry.h"

#include "gui/Window.h"

class Scene;
class AsyncLoader;

/**
 * Base engine
 */
class Engine {
public:
    Engine();
    ~Engine();

    int init();
    int setup();
    void run();

protected:
    virtual int onInit() = 0;
    virtual int onSetup() = 0;
    virtual int onRender(const RenderClock& clock) = 0;
    virtual void onDestroy();

public:
    bool debug;

    std::unique_ptr<Window> window;

    std::shared_ptr<Scene> currentScene;

    RenderClock startClock;

    ShaderRegistry shaders;

protected:
    Assets assets;

    std::shared_ptr<AsyncLoader> asyncLoader;

    std::string title;

    GLState state;
};
