#pragma once

#include <functional>
#include <string>

#include "ki/GL.h"

#include "imgui.h"

#include "asset/Assets.h"
#include "Input.h"

class Engine;

class Window final
{
public:
	Window(Engine& engine);
	~Window();

	bool create();

	void close();
	bool isClosed();

	void setTitle(const std::string& title);

	void processInput(const RenderClock& clock);

	void onWindowResize(int width, int height);
	void onMouseMove(double xpos, double ypos);
	void onMouseButton(int button, int action, int modifiers);
	void onMouseWheel(double xoffset, double yoffset);
private:
	void createGLFWWindow();
	void destroyGLFWWindow();

	void bindGLFWCallbacks();

public:
	const Assets& assets;

	GLFWwindow* glfwWindow{ nullptr };
	int width = 0;
	int height = 0;

	std::unique_ptr<Input> input{ nullptr };

protected:
	Engine& engine;

	std::string title;
};

