#include "Window.h"

#include <iostream>

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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (glfwWindow == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
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
		std::cout << "Failed to initialize GLAD" << std::endl;
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

	// callbacks
	// NOTE KI GLFW does NOT like class functions as callbacks
	// https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback
	// <GLFWwindow*, int, int>
//	auto resize = std::bind(&Engine::on_framebuffer_size, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	//glfwSetFramebufferSizeCallback(window, resize);

	glfwSetFramebufferSizeCallback(glfwWindow, framebuffer_size_callback);

	glfwSetCursorPosCallback(glfwWindow, mouse_callback);
	glfwSetScrollCallback(glfwWindow, scroll_callback);
}

void Window::processInput(float dt)
{
	if (input->isPressed(Key::EXIT)) {
		close();
		return;
	}
	engine.camera.onKey(input, dt);
}

void Window::on_framebuffer_size(int width, int height)
{
	glViewport(0, 0, width, height);
	this->width = width;
	this->height = height;
}

void Window::on_mouse(double xpos, double ypos)
{
	input->handleMouse(xpos, ypos);

	int state = glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)) {
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		engine.camera.onMouseMove(input, input->mouseXoffset, input->mouseYoffset);
	}
	else {
		glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void Window::on_scroll(double xoffset, double yoffset)
{
	engine.camera.onMouseScroll(input, xoffset, yoffset);
}

void Window::framebuffer_size_callback(GLFWwindow* glfwWindow, int width, int height)
{
	Window* window = (Window *)glfwGetWindowUserPointer(glfwWindow);
	window->on_framebuffer_size(width, height);
}

void Window::mouse_callback(GLFWwindow* glfwWindow, double xpos, double ypos)
{
	Window* window = (Window*)glfwGetWindowUserPointer(glfwWindow);
	window->on_mouse(xpos, ypos);
}

void Window::scroll_callback(GLFWwindow* glfwWindow, double xoffset, double yoffset)
{
	Window* window = (Window*)glfwGetWindowUserPointer(glfwWindow);
	window->on_scroll(xoffset, yoffset);
}
