#pragma once

class Engine;

class GuiInit
{
public:
	GuiInit(Engine& engine);
	~GuiInit();

private:
	Engine& engine;
};

