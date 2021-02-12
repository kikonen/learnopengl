#pragma once

#include "GuiWindow.h"


class EditorWindow : public GuiWindow
{
public:
	EditorWindow(Engine& engine);

	void draw(const RenderContext& ctx) override;

};

