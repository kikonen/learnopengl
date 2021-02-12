#pragma once

class Engine;

class FrameInit
{
public:
	FrameInit(Engine& engine);
	~FrameInit();

private:
	Engine& engine;
};

