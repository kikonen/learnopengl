#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Engine.h"
#include "SceneLoader.h"

#include "FrameInit.h"
#include "Frame.h"


class Test6 final : public Engine {
public:
	Test6();

	int onSetup() override;

	SceneLoader* loadScene();

	int onRender(float dt) override;
	void onDestroy() override;
	void processInput(float dt) override;

private:
	Scene* currentScene = nullptr;
	Frame* frame;
	FrameInit* frameInit;
};
