#include "Input.h"

#include "Window.h"

namespace {
    const Modifier modifierKeys[3] = {
        Modifier::SHIFT,
        Modifier::CONTROL,
        Modifier::ALT,
    };
}

Input::Input(Window* window)
    : window(window)
{
    m_keyMappings[Key::EXIT] = new int[] { GLFW_KEY_ESCAPE, 0 };
    m_keyMappings[Key::FULL_SCREEN_TOGGLE] = new int[] { GLFW_KEY_F1, GLFW_KEY_F12, 0 };
    m_keyMappings[Key::FORWARD] = new int[] { GLFW_KEY_W, GLFW_KEY_UP, 0 };
    m_keyMappings[Key::BACKWARD] = new int[] { GLFW_KEY_S, GLFW_KEY_DOWN, 0 };
    m_keyMappings[Key::LEFT] = new int[] { GLFW_KEY_A, GLFW_KEY_LEFT, 0 };
    m_keyMappings[Key::RIGHT] = new int[] { GLFW_KEY_D, GLFW_KEY_RIGHT, 0 };
    m_keyMappings[Key::ROTATE_LEFT] = new int[] { GLFW_KEY_Q, 0 };
    m_keyMappings[Key::ROTATE_RIGHT] = new int[] { GLFW_KEY_E, 0 };
    m_keyMappings[Key::UP] = new int[] { GLFW_KEY_PAGE_UP, 0 };
    m_keyMappings[Key::DOWN] = new int[] { GLFW_KEY_PAGE_DOWN, 0 };

    m_keyMappings[Key::ZOOM_IN] = new int[] { GLFW_KEY_HOME, 0 };
    m_keyMappings[Key::ZOOM_OUT] = new int[] { GLFW_KEY_END, 0 };

    m_modifierMappings[Modifier::SHIFT] = new int[] { GLFW_KEY_LEFT_SHIFT, GLFW_KEY_RIGHT_SHIFT, GLFW_KEY_CAPS_LOCK, 0 };
    m_modifierMappings[Modifier::CONTROL] = new int[] { GLFW_KEY_LEFT_CONTROL, GLFW_KEY_RIGHT_CONTROL, 0 };
    m_modifierMappings[Modifier::ALT] = new int[] { GLFW_KEY_LEFT_ALT, GLFW_KEY_RIGHT_ALT, 0 };

    for (auto& mod : modifierKeys) {
        m_modifierDown[mod] = false;
        m_modifierPressed[mod] = false;
        m_modifierReleased[mod] = false;
    }
}

Input::~Input()
{
    for (auto& e : m_keyMappings) {
        delete[] e.second;
    }
    m_keyMappings.clear();

    for (auto& e : m_modifierMappings) {
        delete[] e.second;
    }
    m_modifierMappings.clear();

}

void Input::prepare()
{
    if (m_prepared) return;
    m_prepared = true;

    updateKeyStates();
}

void Input::updateKeyStates()
{
    for (auto& mod : modifierKeys) {
        bool wasDown = m_modifierDown[mod];
        bool isDown = isModifierDown(mod);
        m_modifierDown[mod] = isDown;
        m_modifierPressed[mod] = isDown && !wasDown;
        m_modifierReleased[mod] = !isDown && wasDown;
    }
}

bool Input::isKeyDown(Key key)
{
    int* code = m_keyMappings[key];
    if (code) {
        while (*code) {
            if (glfwGetKey(window->m_glfwWindow, *code) == GLFW_PRESS) {
                return true;
            }
            code++;
        }
    }
    return false;
}

bool Input::isModifierDown(Modifier modifier) {
    int* code = m_modifierMappings[modifier];
    if (code) {
        while (*code) {
            if (glfwGetKey(window->m_glfwWindow, *code) == GLFW_PRESS) {
                return true;
            }
            code++;
        }
    }
    return false;
}

bool Input::isModifierPressed(Modifier modifier)
{
    return m_modifierPressed[modifier];
}

void Input::onMouseMove(float xpos, float ypos)
{
    if (m_firstMouse) {
        mouseX = xpos;
        mouseX = ypos;
        m_firstMouse = false;
    }

    // NOTE KI Match world axis directions
    mouseXoffset = xpos - mouseX;
    mouseYoffset = mouseY - ypos;

    mouseX = xpos;
    mouseY = ypos;
}

void Input::onMouseButton(int button, int action, int modifiers)
{
}
