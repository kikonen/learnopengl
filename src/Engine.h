#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <strstream>
#include <chrono>
#include <thread>

#include "Input.h"
#include "Camera.h"
#include "Assets.h"

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

    /*
    void render() {
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(mesh->shaderProgram);
        glBindVertexArray(mesh->VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time
    }
    */

    GLFWwindow* createWindow();

    virtual void processInput(float dt);

    // https://stackoverflow.com/questions/31581200/glfw-call-to-non-static-class-function-in-static-key-callback
    void on_framebuffer_size(int width, int height);
    void on_mouse(double xpos, double ypos);
    void on_scroll(double xoffset, double yoffset);

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

public:
    bool debug;
    int throttleFps;
    std::string title;

    Camera camera;

    float accumulatedTime = 0.0f;

    static Engine* current;

    Assets assets;

protected:
    GLFWwindow* window = nullptr;
    Input* input;
private:
    int width;
    int height;
};
