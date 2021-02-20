#pragma once

#include "ki/GL.h"

#include <iostream>

#include "Engine.h"
#include "scene/SceneLoader.h"

#include "gui/FrameInit.h"
#include "gui/Frame.h"


class Test6 final : public Engine {
public:
	Test6();

	int onSetup() override;

	SceneLoader* loadScene();

	int onRender(float dt) override;
	void onDestroy() override;

private:
	Frame* frame;
	FrameInit* frameInit;

	SceneLoader* loader = nullptr;
};
