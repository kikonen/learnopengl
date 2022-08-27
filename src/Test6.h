#pragma once

#include "ki/GL.h"

#include <iostream>

#include "Engine.h"

#include "gui/FrameInit.h"
#include "gui/Frame.h"


class Test6 final : public Engine {
public:
	Test6();

protected:
	int onSetup() override;

	int onRender(const RenderClock& clock) override;
	void onDestroy() override;

private:
	Scene* loadScene();

private:
	Frame* frame;
	FrameInit* frameInit;

};
