#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"
#include "SceneSetup1.h"

#include "FrameInit.h"
#include "Frame.h"


class Test6 final : public Engine {
public:
	Test6();

	int onSetup() override;

	SceneSetup1* setupScene1();

	int onRender(float dt) override;
	void onDestroy() override;
	void processInput(float dt) override;

private:
	SceneSetup1* currentScene = nullptr;
	Frame* frame;
	FrameInit* frameInit;
};
