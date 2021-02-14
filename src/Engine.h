#pragma once

#include <iostream>
#include <fstream>
#include <strstream>
#include <chrono>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Input.h"
#include "Camera.h"
#include "Assets.h"
#include "UBO.h"
#include "Shader.h"

#include "Window.h"


const int FPS_120 = 8;
const int FPS_60 = 16;
const int FPS_30 = 33;
const int FPS_15 = 66;
const int FPS_10 = 100;

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
    int throttleFps;

    Window* window = nullptr;

    Camera camera;

    float accumulatedTime = 0.0f;

    static Engine* current;

    Assets assets;

protected:
    std::string title;
};
