#include "Window.h"

#include "kigl/GLState.h"

#include "util/file.h"

#include "asset/Assets.h"

#include "engine/Engine.h"
#include "engine/InputContext.h"

#include "controller/NodeController.h"

#include "asset/DynamicCubeMap.h"
#include "material/Image.h"

#include "registry/Registry.h"

#include "scene/Scene.h"

#include "Input.h"

namespace
{
    // https://github.com/glfw/glfw/issues/1699
    bool glfw_get_mouse_monitor(
        GLFWmonitor** monitor,
        GLFWwindow* window
    ) {
        bool success = false;

        double cursor_position[2] = { 0 };
        glfwGetCursorPos(window, &cursor_position[0], &cursor_position[1]);

        int window_position[2] = { 0 };
        glfwGetWindowPos(window, &window_position[0], &window_position[1]);

        int monitors_size = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&monitors_size);

        // convert cursor position from window coordinates to screen coordinates
        cursor_position[0] += window_position[0];
        cursor_position[1] += window_position[1];

        for (int i = 0; ((!success) && (i < monitors_size)); ++i)
        {
            int monitor_position[2] = { 0 };
            glfwGetMonitorPos(monitors[i], &monitor_position[0], &monitor_position[1]);

            const GLFWvidmode* monitor_video_mode = glfwGetVideoMode(monitors[i]);

            if (
                (cursor_position[0] < monitor_position[0]) ||
                (cursor_position[0] > (monitor_position[0] + monitor_video_mode->width)) ||
                (cursor_position[1] < monitor_position[1]) ||
                (cursor_position[1] > (monitor_position[1] + monitor_video_mode->height))
                ) {
                *monitor = monitors[i];
                success = true;
            }
        }

        // true: monitor contains the monitor the mouse is on
        // false: monitor is unmodified
        return success;
    }

    bool glfw_get_window_monitor(
        GLFWmonitor** monitor,
        GLFWwindow* window
    ) {
        bool success = false;

        int window_rectangle[4] = { 0 };
        glfwGetWindowPos(window, &window_rectangle[0], &window_rectangle[1]);
        glfwGetWindowSize(window, &window_rectangle[2], &window_rectangle[3]);

        int monitors_size = 0;
        GLFWmonitor** monitors = glfwGetMonitors(&monitors_size);

        GLFWmonitor* closest_monitor = NULL;

        int max_overlap_area = 0;

        for (int i = 0; i < monitors_size; ++i)
        {
            int monitor_position[2] = { 0 };
            glfwGetMonitorPos(monitors[i], &monitor_position[0], &monitor_position[1]);

            const GLFWvidmode* monitor_video_mode = glfwGetVideoMode(monitors[i]);

            int monitor_rectangle[4] = {
                monitor_position[0],
                monitor_position[1],
                monitor_video_mode->width,
                monitor_video_mode->height,
            };

            if (
                !(
                    ((window_rectangle[0] + window_rectangle[2]) < monitor_rectangle[0]) ||
                    (window_rectangle[0] > (monitor_rectangle[0] + monitor_rectangle[2])) ||
                    ((window_rectangle[1] + window_rectangle[3]) < monitor_rectangle[1]) ||
                    (window_rectangle[1] > (monitor_rectangle[1] + monitor_rectangle[3]))
                    )
                ) {
                int intersection_rectangle[4] = { 0 };

                // x, width
                if (window_rectangle[0] < monitor_rectangle[0])
                {
                    intersection_rectangle[0] = monitor_rectangle[0];

                    if ((window_rectangle[0] + window_rectangle[2]) < (monitor_rectangle[0] + monitor_rectangle[2]))
                    {
                        intersection_rectangle[2] = (window_rectangle[0] + window_rectangle[2]) - intersection_rectangle[0];
                    }
                    else
                    {
                        intersection_rectangle[2] = monitor_rectangle[2];
                    }
                }
                else
                {
                    intersection_rectangle[0] = window_rectangle[0];

                    if ((monitor_rectangle[0] + monitor_rectangle[2]) < (window_rectangle[0] + window_rectangle[2]))
                    {
                        intersection_rectangle[2] = (monitor_rectangle[0] + monitor_rectangle[2]) - intersection_rectangle[0];
                    }
                    else
                    {
                        intersection_rectangle[2] = window_rectangle[2];
                    }
                }

                // y, height
                if (window_rectangle[1] < monitor_rectangle[1])
                {
                    intersection_rectangle[1] = monitor_rectangle[1];

                    if ((window_rectangle[1] + window_rectangle[3]) < (monitor_rectangle[1] + monitor_rectangle[3]))
                    {
                        intersection_rectangle[3] = (window_rectangle[1] + window_rectangle[3]) - intersection_rectangle[1];
                    }
                    else
                    {
                        intersection_rectangle[3] = monitor_rectangle[3];
                    }
                }
                else
                {
                    intersection_rectangle[1] = window_rectangle[1];

                    if ((monitor_rectangle[1] + monitor_rectangle[3]) < (window_rectangle[1] + window_rectangle[3]))
                    {
                        intersection_rectangle[3] = (monitor_rectangle[1] + monitor_rectangle[3]) - intersection_rectangle[1];
                    }
                    else
                    {
                        intersection_rectangle[3] = window_rectangle[3];
                    }
                }

                int overlap_area = intersection_rectangle[2] * intersection_rectangle[3];
                if (overlap_area > max_overlap_area)
                {
                    closest_monitor = monitors[i];
                    max_overlap_area = overlap_area;
                }
            }
        }

        if (closest_monitor)
        {
            *monitor = closest_monitor;
            success = true;
        }

        // true: monitor contains the monitor the window is most on
        // false: monitor is unmodified
        return success;
    }
}

Window::Window(
    Engine& engine)
    : m_engine(engine)
{
    const auto& assets = Assets::get();

    m_size = assets.windowSize;
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

const glm::uvec2& Window::getSize()
{
    if (!m_sizeValid) {
        int w;
        int h;
        glfwGetWindowSize(m_glfwWindow, &w, &h);

        m_size = { w, h };

        if (w < 1) w = 1;
        if (h < 1) h = 1;

        m_safeSize = { w, h };

        m_sizeValid = true;
    }
    return m_safeSize;
}

void Window::setTitle(std::string_view title)
{
    m_title = title;
    glfwSetWindowTitle(m_glfwWindow, std::string{ title }.c_str());
}

void Window::toggleFullScreen()
{
    const auto& assets = Assets::get();

    bool fullScreen = glfwGetWindowMonitor(m_glfwWindow) == nullptr;

    if (fullScreen) {
        {
            int x, y;
            int w, h;
            glfwGetWindowPos(m_glfwWindow, &x, &y);
            glfwGetWindowSize(m_glfwWindow, &w, &h);
            m_windowedPos.x = x;
            m_windowedPos.y = y;
            m_windowedSize.x = w;
            m_windowedSize.y = h;
            m_windowedWasMaximized = glfwGetWindowAttrib(m_glfwWindow, GLFW_MAXIMIZED);
        }


        GLFWmonitor* monitor{ nullptr };
        bool success = glfw_get_window_monitor(&monitor, m_glfwWindow);
        if (!success) {
            monitor = glfwGetPrimaryMonitor();
        }

        glm::uvec2 size{ 0 };
        const auto* mode = glfwGetVideoMode(monitor);
        size.x = mode->width;
        size.y = mode->height;

        int monitorPos[2] = { 0 };
        glfwGetMonitorPos(monitor, &monitorPos[0], &monitorPos[1]);

        glfwSetWindowMonitor(
            m_glfwWindow,
            monitor,
            monitorPos[0],
            monitorPos[1],
            size.x,
            size.y,
            GLFW_DONT_CARE);

        m_fullScreen = true;
    }
    else {
        glfwSetWindowMonitor(
            m_glfwWindow,
            nullptr,
            m_windowedPos.x,
            m_windowedPos.y,
            m_windowedSize.x,
            m_windowedSize.y,
            0);
        if (m_windowedWasMaximized) {
            glfwMaximizeWindow(m_glfwWindow);
        }
        m_fullScreen = false;
    }

    m_input->updateKeyStates();

    // https://community.khronos.org/t/glfw-fullscreen-problem/51536
    glfwSwapInterval(assets.glfwSwapInterval);
}

void Window::close()
{
    glfwSetWindowShouldClose(m_glfwWindow, true);
}

bool Window::isClosed() const
{
    return glfwWindowShouldClose(m_glfwWindow);
}

void Window::createGLFWWindow()
{
    const auto& assets = Assets::get();

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

    if (assets.windowMaximized) {
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    }

    KI_INFO("START: WDINDOW CREATE");
    m_glfwWindow = glfwCreateWindow(m_size.x, m_size.y, m_title.c_str(), nullptr, nullptr);
    if (m_glfwWindow == nullptr)
    {
        KI_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return;
    }

    if (assets.windowFullScreen) {
        toggleFullScreen();
    }

    glfwSetWindowUserPointer(m_glfwWindow, this);
    glfwMakeContextCurrent(m_glfwWindow);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwTerminate();
        KI_ERROR("Failed to initialize GLAD");
        return;
    }

    if (!assets.windowIcon.empty()) {
        const auto& texturePath = util::joinPath(
            assets.assetsDir,
            assets.windowIcon);

        Image image{ texturePath, false };
        if (image.load() == 0) {
            GLFWimage images{
                image.m_width,
                image.m_height,
                image.m_data,
            };

            glfwSetWindowIcon(m_glfwWindow, 1, &images);
        }
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
            static_cast<Window*>(glfwGetWindowUserPointer(gw))->onMouseMove(
                static_cast<float>(xpos),
                static_cast<float>(ypos));
        });

    glfwSetMouseButtonCallback(
        m_glfwWindow,
        [](GLFWwindow* gw, int button, int action, int modifiers) {
            static_cast<Window*>(glfwGetWindowUserPointer(gw))->onMouseButton(button, action, modifiers);
        });

    glfwSetScrollCallback(
        m_glfwWindow,
        [](GLFWwindow* gw, double xoffset, double yoffset) {
            static_cast<Window*>(glfwGetWindowUserPointer(gw))->onMouseWheel(
                static_cast<float>(xoffset),
                static_cast<float>(yoffset));
        });
}

void Window::processInput(const InputContext& ctx)
{
    m_input->updateKeyStates();
    m_input->updateMouseState();

    if (m_input->isKeyDown(Key::EXIT)) {
        if (!m_was_EXIT) {
            KI_INFO("INPUT: USER EXIT via [ESCAPE]");
            m_was_EXIT = true;
            close();
            return;
        }
    }
    else {
        m_was_EXIT = false;
    }

    if (m_input->isKeyDown(Key::FULL_SCREEN_TOGGLE)) {
        if (!m_was_FULL_SCREEN_TOGGLE) {
            KI_INFO("INPUT: FULL_SCREEN_TOGGLE");
            m_was_FULL_SCREEN_TOGGLE = true;
            toggleFullScreen();
            return;
        }
    }
    else {
        m_was_FULL_SCREEN_TOGGLE = false;
    }

    auto* nodeControllers = m_engine.m_currentScene->getActiveNodeControllers();
    auto* cameraControllers = m_engine.m_currentScene->getActiveCameraControllers();

    {
        if (nodeControllers) {
            for (auto* controller : *nodeControllers) {
                controller->processInput(ctx);
            }
        }
    }
    {
        if (cameraControllers && cameraControllers != nodeControllers) {
            for (auto* controller : *cameraControllers) {
                controller->processInput(ctx);
            }
        }
    }
}

void Window::onWindowResize(int width, int height)
{
    auto& state = kigl::GLState::get();

    glViewport(0, 0, width, height);

    m_size = { width, height };
    m_sizeValid = false;

    state.clear();
}

void Window::onMouseMove(float xpos, float ypos)
{
    m_input->onMouseMove(xpos, ypos);

    if (m_input->isMouseCaptured()) {
        glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else {
        glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void Window::onMouseButton(int button, int action, int modifiers)
{
}

void Window::onMouseWheel(float xoffset, float yoffset)
{
    m_input->onMouseWheel(xoffset, yoffset);

    const auto& assets = Assets::get();

    const InputContext ctx{
        m_engine.getClock(),
        m_engine.getRegistry(),
        m_input.get(),
    };

    auto* nodeControllers = m_engine.m_currentScene->getActiveNodeControllers();
    auto* cameraControllers = m_engine.m_currentScene->getActiveCameraControllers();

    {
        if (nodeControllers) {
            for (auto* controller : *nodeControllers) {
                controller->onMouseWheel(ctx, xoffset, yoffset);
            }
        }
    }
    {
        if (cameraControllers && cameraControllers != nodeControllers) {
            for (auto* controller : *cameraControllers) {
                controller->onMouseWheel(ctx, xoffset, yoffset);
            }
        }
    }
}
