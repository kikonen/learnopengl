#include "FrameInit.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Engine.h"

FrameInit::FrameInit(Engine& engine)
	: engine(engine)
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(engine.window, true);
	ImGui_ImplOpenGL3_Init(engine.glsl_version.c_str());
	// Setup Dear ImGui style
	ImGui::StyleColorsLight();
}

FrameInit::~FrameInit()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}
