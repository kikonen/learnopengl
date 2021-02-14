#pragma once

#include <map>

class Window;

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
class Input final
{
public:
	Input(Window* window);
	~Input();

	bool isKeyPressed(Key key);
	void onMouseMove(double xpos, double ypos);
	void onMouseButton(int button, int action, int modifiers);

public:
	float mouseX = 0;
	float mouseY = 0;

	float mouseXoffset = 0;
	float mouseYoffset = 0;

	Window* window;
private:

	std::map<Key, int*> mapping;

	bool firstMouse = true;

};

