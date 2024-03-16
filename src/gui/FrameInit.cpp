#include "FrameInit.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "util/Util.h"

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

void FrameInit::prepare(const PrepareContext& ctx)
{
    const auto& assets = Assets::get();

    m_fontSize = assets.imGuiFontSize;
    m_fontPath = util::joinPath(
        assets.assetsDir,
        assets.imGuiFontPath);

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.Fonts->AddFontFromFileTTF(m_fontPath.c_str(), m_fontSize);
}
