#include "Window.h"

#include <iostream>

#include "controller/NodeController.h"
#include "scene/Scene.h"
#include "Engine.h"

Window::Window(Engine& engine, const Assets& assets)
    : m_engine(engine), assets(assets)
{
    m_width = 800;
    m_height = 600;
    m_title = "GL test";

    m_input = std::make_unique<Input>(this);
}

Window::~Window()
{
    destroyGLFWWindow();
}

bool Window::create()
{
    createGLFWWindow();
    m_input->prepare();
    return m_glfwWindow != nullptr;
}

void Window::setTitle(const std::string& title)
{
    m_title = title;
    glfwSetWindowTitle(m_glfwWindow, title.c_str());
}

void Window::close()
{
    glfwSetWindowShouldClose(m_glfwWindow, true);
}

bool Window::isClosed()
{
    return glfwWindowShouldClose(m_glfwWindow);
}

void Window::createGLFWWindow()
{
    // glfw: initialize and configure
    // ------------------------------
    KI_INFO("START: GLFW INIT");
    glfwInit();
    KI_INFO("DONE: GLFW INIT");

    if (assets.glDebug) {
        // NOTE KI MUST be after glfwInit BUT before glfwWindow creat4e
        // https://learnopengl.com/in-practice/debugging
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    }

    if (assets.glNoError) {
        // https://www.khronos.org/opengl/wiki/OpenGL_Error#No_error_contexts
        glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, assets.glsl_version[0]);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, assets.glsl_version[1]);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


//#ifdef __APPLE__
//    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

    KI_INFO("START: WDINDOW CREATE");
    m_glfwWindow = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (m_glfwWindow == nullptr)
    {
        KI_ERROR_SB("Failed to create GLFW window");
        glfwTerminate();
        return;
    }

    glfwSetWindowUserPointer(m_glfwWindow, this);
    glfwMakeContextCurrent(m_glfwWindow);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        KI_ERROR_SB("Failed to initialize GLAD");
        return;
    }

    bindGLFWCallbacks();
}

void Window::destroyGLFWWindow()
{
    glfwTerminate();
}

void Window::bindGLFWCallbacks()
{
    // NOTE KI GLFW does NOT like class functions as callbacks
    // https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback
    // https://stackoverflow.com/questions/31581200/glfw-call-to-non-static-class-function-in-static-key-callback

    glfwSetFramebufferSizeCallback(
        m_glfwWindow,
        [](GLFWwindow* gw, int width, int height) {
        static_cast<Window*>(glfwGetWindowUserPointer(gw))->onWindowResize(width, height);
    });

    glfwSetCursorPosCallback(
        m_glfwWindow,
        [](GLFWwindow* gw, double xpos, double ypos) {
            static_cast<Window*>(glfwGetWindowUserPointer(gw))->onMouseMove(xpos, ypos);
        });

    glfwSetMouseButtonCallback(
        m_glfwWindow,
        [](GLFWwindow* gw, int button, int action, int modifiers) {
            static_cast<Window*>(glfwGetWindowUserPointer(gw))->onMouseButton(button, action, modifiers);
        });

    glfwSetScrollCallback(
        m_glfwWindow,
        [](GLFWwindow* gw, double xoffset, double yoffset) {
            static_cast<Window*>(glfwGetWindowUserPointer(gw))->onMouseWheel(xoffset, yoffset);
        });
}

void Window::processInput(const ki::RenderClock& clock)
{
    m_input->updateKeyStates();

    if (m_input->isKeyDown(Key::EXIT)) {
        KI_INFO("INPUT: USER EXIT via [ESCAPE]");
        close();
        return;
    }

    auto* controller = m_engine.m_currentScene->getCameraController();
    if (controller) {
        controller->onKey(m_input.get(), clock);
    }
}

void Window::onWindowResize(int width, int height)
{
    glViewport(0, 0, width, height);
    m_width = width;
    m_height = height;
}

void Window::onMouseMove(double xpos, double ypos)
{
    m_input->onMouseMove(xpos, ypos);

    bool isAlt = m_input->isModifierDown(Modifier::ALT);
    int state = glfwGetMouseButton(m_glfwWindow, GLFW_MOUSE_BUTTON_LEFT);

    if ((isAlt || state == GLFW_PRESS) && (!assets.useIMGUI || !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))) {
        glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        auto* controller = m_engine.m_currentScene->getCameraController();
        if (controller) {
            controller->onMouseMove(m_input.get(), m_input->mouseXoffset, m_input->mouseYoffset);
        }
    }
    else {
        glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Window::onMouseButton(int button, int action, int modifiers)
{
    m_input->onMouseButton(button, action, modifiers);
}

void Window::onMouseWheel(double xoffset, double yoffset)
{
    auto* controller = m_engine.m_currentScene->getCameraController();
    if (controller) {
        controller->onMouseScroll(m_input.get(), xoffset, yoffset);
    }
}
