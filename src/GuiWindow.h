#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "RenderContext.h"

class Engine;

class GuiWindow
{
public:
	GuiWindow(Engine& engine);
	~GuiWindow();

	virtual void prepare();
	virtual void bind(const RenderContext& ctx);
	virtual void draw(const RenderContext& ctx) = 0;

	virtual void render(const RenderContext& ctx);

private:
	Engine& engine;
};

