#pragma once

#include "ki/GL.h"

#include "asset/GLState.h"
#include "asset/Assets.h"
#include "gui/Window.h"

class Scene;

const float FPS_120 = 8;
const float FPS_60 = 16;
const float FPS_30 = 33;
const float FPS_15 = 66;
const float FPS_10 = 100;

/**
 * Base engine 
 */
class Engine {
public:
    Engine();
    ~Engine();

    int init();
    void run();

protected:
    virtual int onSetup() = 0;
    virtual int onRender(const RenderClock& clock) = 0;
    virtual void onDestroy();

public:
    bool debug;
    float throttleFps;
    bool useIMGUI = false;

    Window* window = nullptr;

    Scene* currentScene = nullptr;

    RenderClock startClock;

    static Engine* current;

    Assets assets;

protected:
    std::string title;

    GLState state;
};
