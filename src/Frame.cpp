#include "Frame.h"

#include "Engine.h"

Frame::Frame(Engine& engine)
	: engine(engine)
{
}

Frame::~Frame()
{
}

void Frame::prepare()
{
}

void Frame::bind(const RenderContext& ctx)
{
	// feed inputs to dear imgui, start new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Frame::render(const RenderContext& ctx)
{
	// Render dear imgui into screen
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


