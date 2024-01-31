#include "FrameInit.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "asset/Assets.h"

#include "engine/Engine.h"

FrameInit::FrameInit(Window& window)
    : window(window)
{
    const auto& assets = Assets::get();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window.m_glfwWindow, true);
    ImGui_ImplOpenGL3_Init(assets.glsl_version_str.c_str());
    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
}

FrameInit::~FrameInit()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
