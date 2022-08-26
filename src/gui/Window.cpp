#include "Window.h"

#include <iostream>

#include "component/Camera.h"
#include "scene/Scene.h"
#include "Engine.h"

Window::Window(Engine& engine)
	: engine(engine),
	assets(engine.assets)
{
	width = 800;
	height = 600;
	title = "GL test";

	input = new Input(this);
}

Window::~Window()
{
	destroyGLFWWindow();
	delete input;
}

bool Window::create()
{
	createGLFWWindow();
	input->prepare();
	return glfwWindow != nullptr;
}

void Window::setTitle(const std::string& title)
{
	this->title = title;
	glfwSetWindowTitle(glfwWindow, title.c_str());
}

void Window::close()
{
	glfwSetWindowShouldClose(glfwWindow, true);
}

bool Window::isClosed()
{
	return glfwWindowShouldClose(glfwWindow);
}

void Window::createGLFWWindow()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, assets.glsl_version[0]);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, assets.glsl_version[1]);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

	glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (glfwWindow == nullptr)
	{
		KI_ERROR_SB("Failed to create GLFW window");
		glfwTerminate();
		return;
	}

	glfwSetWindowUserPointer(glfwWindow, this);
	glfwMakeContextCurrent(glfwWindow);

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
		glfwWindow, 
		[](GLFWwindow* gw, int width, int height) {
		static_cast<Window*>(glfwGetWindowUserPointer(gw))->onWindowResize(width, height);
	});

	glfwSetCursorPosCallback(
		glfwWindow,
		[](GLFWwindow* gw, double xpos, double ypos) {
			static_cast<Window*>(glfwGetWindowUserPointer(gw))->onMouseMove(xpos, ypos);
		});

	glfwSetMouseButtonCallback(
		glfwWindow,
		[](GLFWwindow* gw, int button, int action, int modifiers) {
			static_cast<Window*>(glfwGetWindowUserPointer(gw))->onMouseButton(button, action, modifiers);
		});

	glfwSetScrollCallback(
		glfwWindow,
		[](GLFWwindow* gw, double xoffset, double yoffset) {
			static_cast<Window*>(glfwGetWindowUserPointer(gw))->onMouseWheel(xoffset, yoffset);
		});
}

void Window::processInput(const RenderClock& clock)
{
	input->updateKeyStates();

	if (input->isKeyDown(Key::EXIT)) {
		close();
		return;
	}

	Camera* camera = engine.currentScene->getCamera();
	if (camera) {
		camera->onKey(input, clock);
	}
}

void Window::onWindowResize(int width, int height)
{
	glViewport(0, 0, width, height);
	this->width = width;
	this->height = height;
}

void Window::onMouseMove(double xpos, double ypos)
{
	input->onMouseMove(xpos, ypos);

	bool isAlt = input->isModifierDown(Modifier::ALT);
	int state = glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_LEFT);

	if ((isAlt || state == GLFW_PRESS) && (!engine.useIMGUI || !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))) {
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		Camera* camera = engine.currentScene->getCamera();
		if (camera) {
			camera->onMouseMove(input, input->mouseXoffset, input->mouseYoffset);
		}
	}
	else {
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void Window::onMouseButton(int button, int action, int modifiers)
{
	input->onMouseButton(button, action, modifiers);
}

void Window::onMouseWheel(double xoffset, double yoffset)
{
	Camera* camera = engine.currentScene->getCamera();
	if (camera) {
		camera->onMouseScroll(input, xoffset, yoffset);
	}
}
