#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "scene/RenderContext.h"

#include "gui/Window.h"

class Frame
{
public:
	Frame(Window& window);
	~Frame();

	virtual void prepare();
	virtual void bind(const RenderContext& ctx);
	virtual void draw(const RenderContext& ctx) = 0;

	virtual void render(const RenderContext& ctx);

private:
	Window& window;
};

