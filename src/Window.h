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

	// https://stackoverflow.com/questions/31581200/glfw-call-to-non-static-class-function-in-static-key-callback
	void on_framebuffer_size(int width, int height);
	void on_mouse(double xpos, double ypos);
	void on_scroll(double xoffset, double yoffset);

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

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

