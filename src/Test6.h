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

	int onRender(const RenderClock& clock) override;
	void onDestroy() override;

private:
	Frame* frame;
	FrameInit* frameInit;

	SceneLoader* loader = nullptr;

	bool useIMGUI = false;
};
