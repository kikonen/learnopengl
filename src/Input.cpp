#include "Input.h"

#include "Window.h"

Input::Input(Window* window) 
	: window(window)
{
	mapping[EXIT] = new int[] { GLFW_KEY_ESCAPE, 0 };
	mapping[FORWARD] = new int[] { GLFW_KEY_W, GLFW_KEY_UP, 0 };
	mapping[BACKWARD] = new int[] { GLFW_KEY_S, GLFW_KEY_DOWN, 0 };
	mapping[LEFT] = new int[] { GLFW_KEY_A, GLFW_KEY_LEFT, 0 };
	mapping[RIGHT] = new int[] { GLFW_KEY_D, GLFW_KEY_RIGHT, 0 };
	mapping[ROTATE_LEFT] = new int[] { GLFW_KEY_Q, 0 };
	mapping[ROTATE_RIGHT] = new int[] { GLFW_KEY_E, 0 };
	mapping[UP] = new int[] { GLFW_KEY_PAGE_UP, 0 };
	mapping[DOWN] = new int[] { GLFW_KEY_PAGE_DOWN, 0 };

	mapping[ZOOM_IN] = new int[] { GLFW_KEY_HOME, 0 };
	mapping[ZOOM_OUT] = new int[] { GLFW_KEY_END, 0 };

	modifiers[SHIFT] = new int[] { GLFW_KEY_LEFT_SHIFT, GLFW_KEY_RIGHT_SHIFT, GLFW_KEY_CAPS_LOCK, 0 };
	modifiers[CONTROL] = new int[] { GLFW_KEY_LEFT_CONTROL, GLFW_KEY_RIGHT_CONTROL, 0 };
	modifiers[ALT] = new int[] { GLFW_KEY_LEFT_ALT, GLFW_KEY_RIGHT_ALT, 0 };
}

Input::~Input()
{
}

bool Input::isKeyPressed(Key key)
{
	int* code = mapping[key];
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

bool Input::isModifier(Modifier modifier) {
	int* code = modifiers[modifier];
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
