#pragma once

#include "gui/Frame.h"


class EditorFrame : public Frame
{
public:
	EditorFrame(Window& window);

	void draw(const RenderContext& ctx) override;

};

