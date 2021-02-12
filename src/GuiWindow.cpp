#include "GuiWindow.h"

#include "Engine.h"

GuiWindow::GuiWindow(Engine& engine)
	: engine(engine)
{
}

GuiWindow::~GuiWindow()
{
}

void GuiWindow::prepare()
{
}

void GuiWindow::bind(const RenderContext& ctx)
{
	// feed inputs to dear imgui, start new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

