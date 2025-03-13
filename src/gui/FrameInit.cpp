#include "FrameInit.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "util/util.h"
#include "util/file.h"

#include "asset/Assets.h"

#include "engine/Engine.h"

FrameInit::FrameInit(Window& window)
    : m_window(window),
    m_fontSize{ 18 },
    m_fontPath{ "fonts/Vera.ttf" }
{
    const auto& assets = Assets::get();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    {
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= 0
            //| ImGuiConfigFlags_DockingEnable
            //| ImGuiConfigFlags_NavEnableKeyboard
            | 0;
        io.ConfigDebugHighlightIdConflicts = true;
    }
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window.m_glfwWindow, true);
    ImGui_ImplOpenGL3_Init(assets.glsl_version_str.c_str());
    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();
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
