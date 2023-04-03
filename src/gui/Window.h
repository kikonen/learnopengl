#pragma once

#include <functional>
#include <string>

#include "ki/GL.h"
#include "ki/RenderClock.h"

#include "imgui.h"

#include "asset/Assets.h"

class Input;
class Engine;

class Window final
{
public:
    Window(Engine& engine, const Assets& assets);
    ~Window();

    bool create();

    void close();
    bool isClosed();

    void setTitle(const std::string& title);

    void processInput(const ki::RenderClock& clock);

    void onWindowResize(int width, int height);
    void onMouseMove(double xpos, double ypos);
    void onMouseButton(int button, int action, int modifiers);
    void onMouseWheel(double xoffset, double yoffset);
private:
    void createGLFWWindow();
    void destroyGLFWWindow();

    void bindGLFWCallbacks();

public:
    const Assets& assets;

    GLFWwindow* m_glfwWindow{ nullptr };
    int m_width = 0;
    int m_height = 0;

    std::unique_ptr<Input> m_input{ nullptr };

protected:
    Engine& m_engine;

    std::string m_title;
};

