#pragma once

#include <map>
#include <GLFW/glfw3.h>

enum Key {
	EXIT,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	FORWARD,
	BACKWARD,
	ROTATE_LEFT,
	ROTATE_RIGHT,
	ZOOM_IN,
	ZOOM_OUT,
};

//
// Handle keyboard mapping
//
class Input
{
public:
	Input(GLFWwindow* window);
	~Input();

	bool isPressed(Key key);
	void handleMouse(double xpos, double ypos);

public:
	float mouseX = 0;
	float mouseY = 0;

	float mouseXoffset = 0;
	float mouseYoffset = 0;

private:
	GLFWwindow* window;

	std::map<Key, int*> mapping;

	bool firstMouse = true;

};

