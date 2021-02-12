#pragma once

#include "Frame.h"


class EditorFrame : public Frame
{
public:
	EditorFrame(Engine& engine);

	void draw(const RenderContext& ctx) override;

};

