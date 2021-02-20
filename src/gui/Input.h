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

enum Modifier {
	SHIFT,
	CONTROL,
	ALT,
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
	bool isModifier(Modifier modifier);

	void onMouseMove(double xpos, double ypos);
	void onMouseButton(int button, int action, int modifiers);

public:
	double mouseX = 0;
	double mouseY = 0;

	double mouseXoffset = 0;
	double mouseYoffset = 0;

	Window* window;
private:

	std::map<Key, int*> mapping;

	std::map<Modifier, int*> modifiers;

	bool firstMouse = true;

};

