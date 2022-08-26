#pragma once

#include <map>

class Window;

enum class Key {
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

enum class Modifier {
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

	void prepare();

	void updateKeyStates();

	bool isKeyDown(Key key);
	bool isModifierDown(Modifier modifier);

	bool isModifierPressed(Modifier modifier);

	void onMouseMove(double xpos, double ypos);
	void onMouseButton(int button, int action, int modifiers);

public:
	double mouseX = 0;
	double mouseY = 0;

	double mouseXoffset = 0;
	double mouseYoffset = 0;

	Window* window;

private:
	std::map<Key, int*> keyMappings;

	std::map<Modifier, int*> modifierMappings;

	std::map<Modifier, bool> modifierDown;
	std::map<Modifier, bool> modifierPressed;
	std::map<Modifier, bool> modifierReleased;

	bool firstMouse = true;

};

