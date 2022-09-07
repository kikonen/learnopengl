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
    keyMappings[Key::EXIT] = new int[] { GLFW_KEY_ESCAPE, 0 };
    keyMappings[Key::FORWARD] = new int[] { GLFW_KEY_W, GLFW_KEY_UP, 0 };
    keyMappings[Key::BACKWARD] = new int[] { GLFW_KEY_S, GLFW_KEY_DOWN, 0 };
    keyMappings[Key::LEFT] = new int[] { GLFW_KEY_A, GLFW_KEY_LEFT, 0 };
    keyMappings[Key::RIGHT] = new int[] { GLFW_KEY_D, GLFW_KEY_RIGHT, 0 };
    keyMappings[Key::ROTATE_LEFT] = new int[] { GLFW_KEY_Q, 0 };
    keyMappings[Key::ROTATE_RIGHT] = new int[] { GLFW_KEY_E, 0 };
    keyMappings[Key::UP] = new int[] { GLFW_KEY_PAGE_UP, 0 };
    keyMappings[Key::DOWN] = new int[] { GLFW_KEY_PAGE_DOWN, 0 };

    keyMappings[Key::ZOOM_IN] = new int[] { GLFW_KEY_HOME, 0 };
    keyMappings[Key::ZOOM_OUT] = new int[] { GLFW_KEY_END, 0 };

    modifierMappings[Modifier::SHIFT] = new int[] { GLFW_KEY_LEFT_SHIFT, GLFW_KEY_RIGHT_SHIFT, GLFW_KEY_CAPS_LOCK, 0 };
    modifierMappings[Modifier::CONTROL] = new int[] { GLFW_KEY_LEFT_CONTROL, GLFW_KEY_RIGHT_CONTROL, 0 };
    modifierMappings[Modifier::ALT] = new int[] { GLFW_KEY_LEFT_ALT, GLFW_KEY_RIGHT_ALT, 0 };

    for (auto mod : modifierKeys) {
        modifierDown[mod] = false;
        modifierPressed[mod] = false;
        modifierReleased[mod] = false;
    }
}

Input::~Input()
{
    for (auto& e : keyMappings) {
        delete e.second;
    }
    keyMappings.clear();

    for (auto& e : modifierMappings) {
        delete e.second;
    }
    modifierMappings.clear();

}

void Input::prepare()
{
    updateKeyStates();
}

void Input::updateKeyStates()
{
    for (auto mod : modifierKeys) {
        bool wasDown = modifierDown[mod];
        bool isDown = isModifierDown(mod);
        modifierDown[mod] = isDown;
        modifierPressed[mod] = isDown && !wasDown;
        modifierReleased[mod] = !isDown && wasDown;
    }
}

bool Input::isKeyDown(Key key)
{
    int* code = keyMappings[key];
    if (code) {
        while (*code) {
            if (glfwGetKey(window->glfwWindow, *code) == GLFW_PRESS) {
                return true;
            }
            *code++;
        }
    }
    return false;
}

bool Input::isModifierDown(Modifier modifier) {
    int* code = modifierMappings[modifier];
    if (code) {
        while (*code) {
            if (glfwGetKey(window->glfwWindow, *code) == GLFW_PRESS) {
                return true;
            }
            *code++;
        }
    }
    return false;
}

bool Input::isModifierPressed(Modifier modifier)
{
    return modifierPressed[modifier];
}

void Input::onMouseMove(double xpos, double ypos)
{
    if (firstMouse) {
        mouseX = xpos;
        mouseX = ypos;
        firstMouse = false;
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
