#pragma once

#include <functional>
#include <string>

#include "ki/RenderClock.h"

#include "kigl/kigl.h"

#include "imgui.h"

#include "asset/Assets.h"


class Input;
class Engine;
class GLState;

class Window final
{
public:
    Window(
        Engine& engine,
        GLState& state,
        const Assets& assets);
    ~Window();

    bool create();

    const glm::uvec2& getSize();

    void close();
    bool isClosed();

    void setTitle(std::string_view title);

    void processInput(const ki::RenderClock& clock);

    void onWindowResize(int width, int height);
    void onMouseMove(float xpos, float ypos);
    void onMouseButton(int button, int action, int modifiers);
    void onMouseWheel(float xoffset, float yoffset);
private:
    void createGLFWWindow();
    void destroyGLFWWindow();

    void bindGLFWCallbacks();

public:
    const Assets& m_assets;

    GLState& m_state;

    GLFWwindow* m_glfwWindow{ nullptr };

    std::unique_ptr<Input> m_input{ nullptr };

protected:
    Engine& m_engine;

    bool m_sizeValid{ false };
    glm::uvec2 m_size{ 0 } ;
    glm::uvec2 m_safeSize{ 1 };

    std::string m_title;
};

