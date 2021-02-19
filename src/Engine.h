#pragma once

#include "ki/GL.h"

#include "Assets.h"
#include "Window.h"

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

    virtual int onSetup() = 0;
    virtual int onRender(float dt) = 0;
    virtual void onDestroy();

public:
    bool debug;
    float throttleFps;

    Window* window = nullptr;

    Scene* currentScene = nullptr;

    float accumulatedSecs = 0.0f;

    static Engine* current;

    Assets assets;

protected:
    std::string title;
};
