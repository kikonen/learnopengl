#pragma once

#include "ki/GL.h"

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

private:
	Frame* frame;
	FrameInit* frameInit;

	SceneLoader* loader = nullptr;
};
