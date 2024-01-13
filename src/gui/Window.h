#pragma once

#include <functional>
#include <string>

#include "gui/Input.h"

#include "ki/RenderClock.h"

#include "kigl/kigl.h"

#include "imgui.h"

struct InputContext;

class Engine;

class Window final
{
public:
    Window(
        Engine& engine);
    ~Window();

    Engine& getEngine() const noexcept {
        return m_engine;
    }

    bool create();

    const glm::uvec2& getSize();

    bool isFullScreen() const {
        return m_fullScreen;
    }

    void close();
    bool isClosed() const;

    void setTitle(std::string_view title);

    void toggleFullScreen();

    void processInput(const InputContext& ctx);

    void onWindowResize(int width, int height);
    void onMouseMove(float xpos, float ypos);
    void onMouseButton(int button, int action, int modifiers);
    void onMouseWheel(float xoffset, float yoffset);
private:
    void createGLFWWindow();
    void destroyGLFWWindow();

    void bindGLFWCallbacks();

public:
    GLFWwindow* m_glfwWindow{ nullptr };

    std::unique_ptr<Input> m_input{ nullptr };

    bool m_was_EXIT{ false };
    bool m_was_FULL_SCREEN_TOGGLE{ false };

protected:
    Engine& m_engine;

    bool m_sizeValid{ false };
    glm::uvec2 m_size{ 0 };
    glm::uvec2 m_safeSize{ 1 };

    glm::uvec2 m_windowedPos{ 0 };
    glm::uvec2 m_windowedSize{ 0 };
    bool m_windowedWasMaximized{ false };

    bool m_fullScreen{ false };

    std::string m_title;
};

