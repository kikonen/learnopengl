#include "Input.h"

#include "Window.h"

#include "util/Util.h"

namespace {
}

Input::Input(Window* window)
    : window(window)
{
    addKeyMapping(
        Key::EXIT,
        {
            GLFW_KEY_ESCAPE,
        });

    addKeyMapping(
        Key::FULL_SCREEN_TOGGLE,
        {
            GLFW_KEY_F10,
            GLFW_KEY_F11,
        });
    addKeyMapping(
        Key::FORWARD,
        {
            GLFW_KEY_W,
            GLFW_KEY_UP,
        });
    addKeyMapping(
        Key::BACKWARD,
        {
            GLFW_KEY_S,
            GLFW_KEY_DOWN,
        });
    addKeyMapping(
        Key::LEFT,
        {
            GLFW_KEY_A,
            GLFW_KEY_LEFT,
        });
    addKeyMapping(
        Key::RIGHT,
        {
            GLFW_KEY_D,
            GLFW_KEY_RIGHT,
        });
    addKeyMapping(
        Key::ROTATE_LEFT,
        {
            GLFW_KEY_Q,
        });
    addKeyMapping(
        Key::ROTATE_RIGHT,
        {
            GLFW_KEY_E,
        });
    addKeyMapping(
        Key::UP,
        {
            GLFW_KEY_PAGE_UP,
            GLFW_KEY_R,
        });
    addKeyMapping(
        Key::DOWN,
        {
            GLFW_KEY_PAGE_DOWN,
            GLFW_KEY_F,
        });

    addKeyMapping(
        Key::ZOOM_IN,
        {
            GLFW_KEY_HOME,
            GLFW_KEY_1,
            GLFW_KEY_EQUAL,
        });

    addKeyMapping(
        Key::ZOOM_OUT,
        {
            GLFW_KEY_END,
            GLFW_KEY_2,
            GLFW_KEY_MINUS,
        });
    addKeyMapping(
        Key::MOUSE_LOCK,
        {
            GLFW_KEY_SPACE,
        });

    addModifierMapping(
        Modifier::SHIFT,
        {
            GLFW_KEY_LEFT_SHIFT,
            GLFW_KEY_RIGHT_SHIFT,
            GLFW_KEY_CAPS_LOCK,
        });
    addModifierMapping(
        Modifier::CONTROL,
        {
            GLFW_KEY_LEFT_CONTROL,
            GLFW_KEY_RIGHT_CONTROL,
        });
    addModifierMapping(
        Modifier::ALT,
        {
            GLFW_KEY_LEFT_ALT,
            GLFW_KEY_RIGHT_ALT,
        });


    m_keyStates.resize(util::as_integer(Key::KEY_COUNT));
    for (auto& state : m_keyStates)
    {
        state.wasDown = false;
        state.down = false;
        state.released = false;
        state.pressed = false;
    }

    m_modifierStates.resize(util::as_integer(Modifier::KEY_COUNT));
    for (auto& state : m_modifierStates)
    {
        state.wasDown = false;
        state.down = false;
        state.released = false;
        state.pressed = false;
    }
}

Input::~Input()
{
}

void Input::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    updateKeyStates();
}

bool Input::updateKeyStates()
{
    bool keyDown = false;

    auto* glfwWindow = window->m_glfwWindow;

    for (auto& state : m_keyStates)
    {
        state.wasDown = state.down;
        state.down = false;
    }

    for (const auto& mapping : m_keyMapping)
    {
        auto& state = m_keyStates[util::as_integer(mapping.key)];
        state.down |= glfwGetKey(glfwWindow, mapping.code) == GLFW_PRESS;
    }

    for (auto& state : m_keyStates)
    {
        state.pressed = state.down && !state.wasDown;
        state.released = !state.down && state.wasDown;
    }

    return keyDown;
}

bool Input::updateModifierStates()
{
    bool keyDown = false;

    auto* glfwWindow = window->m_glfwWindow;

    for (auto& state : m_modifierStates)
    {
        state.wasDown = state.down;
        state.down = false;
    }

    for (const auto& mapping : m_modifierMapping)
    {
        auto& state = m_modifierStates[util::as_integer(mapping.key)];
        state.down |= glfwGetKey(glfwWindow, mapping.code) == GLFW_PRESS;
    }

    for (auto& state : m_modifierStates)
    {
        state.pressed = state.down && !state.wasDown;
        state.released = !state.down && state.wasDown;
    }

    return keyDown;
}

void Input::updateMouseState()
{
    double xpos, ypos;
    glfwGetCursorPos(window->m_glfwWindow, &xpos, &ypos);

    if (mouseHasPosition && isMouseCaptured()) {
        // NOTE KI Match world axis directions
        mouseRelativeX = static_cast<float>(xpos) - mouseX;
        mouseRelativeY = mouseY - static_cast<float>(ypos);

        mousePreviousX = mouseX;
        mousePreviousY = mouseY;
    }
    else {
        // NOTE KI reset relative offset if mouse has been released
        mouseRelativeX = 0.f;
        mouseRelativeY = 0.f;
    }

    mouseX = static_cast<float>(xpos);
    mouseY = static_cast<float>(ypos);
    mouseHasPosition = true;
}

void Input::addKeyMapping(
    Key key,
    const std::vector<int>& codes
)
{
    for (int code : codes)
    {
        m_keyMapping.emplace_back(key, code);
    }
}

void Input::addModifierMapping(
    Modifier key,
    const std::vector<int>& codes
)
{
    for (int code : codes)
    {
        m_modifierMapping.emplace_back(key, code);
    }
}

bool Input::isKeyDown(Key key) const noexcept
{
    if (!allowKeyboard()) return false;
    return m_keyStates[util::as_integer(key)].down;
}

bool Input::isModifierDown(Modifier modifier) const noexcept
{
    if (!allowKeyboard()) return false;
    return m_modifierStates[util::as_integer(modifier)].down;
}

bool Input::isModifierPressed(Modifier modifier) const noexcept
{
    if (!allowKeyboard()) return false;
    return m_modifierStates[util::as_integer(modifier)].pressed;
}

bool Input::isMouseButtonPressed(int button) const noexcept
{
    return glfwGetMouseButton(window->m_glfwWindow, button);
}

bool Input::isHighPrecisionMode() const noexcept {
    return isModifierDown(Modifier::CONTROL);
}

bool Input::isMouseCaptured() const noexcept
{
    return allowMouse() &&
        (isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) ||
        isKeyDown(Key::MOUSE_LOCK));
        //isModifierDown(Modifier::ALT);
}

void Input::onMouseMove(float xpos, float ypos)
{
    return;

    if (m_firstMouse) {
        mouseX = xpos;
        mouseX = ypos;
        m_firstMouse = false;
    }

    // NOTE KI Match world axis directions
    mouseRelativeX = xpos - mouseX;
    mouseRelativeY = mouseY - ypos;

    mouseX = xpos;
    mouseY = ypos;
}

void Input::onMouseWheel(float xoffset, float yoffset)
{
    mouseWheelXOffset = xoffset;
    mouseWheelYOffset = yoffset;
}

InputState Input::getState() const
{
    return {
        isModifierDown(Modifier::CONTROL),
        isModifierDown(Modifier::SHIFT),
        isModifierDown(Modifier::ALT),
        glfwGetMouseButton(window->m_glfwWindow, GLFW_MOUSE_BUTTON_LEFT) != 0,
        glfwGetMouseButton(window->m_glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) != 0
    };
}
