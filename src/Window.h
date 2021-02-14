#pragma once

#include <functional>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"

#include "Assets.h"
#include "Input.h"

class Engine;

class Window
{
public:
	Window(Engine& engine);
	~Window();

	bool create();

	void close();
	bool isClosed();

	void setTitle(const std::string& title);

	void processInput(float dt);

	void onWindowResize(int width, int height);
	void onMouseMove(double xpos, double ypos);
	void onMouseWheel(double xoffset, double yoffset);
private:
	void createGLFWWindow();
	void destroyGLFWWindow();

	void bindGLFWCallbacks();

public:
	GLFWwindow* glfwWindow = nullptr;
	int width;
	int height;

	std::string glsl_version = "#version 330";

protected:
	const Assets& assets;
	Engine& engine;
	Input* input = nullptr;

	std::string title;
};

