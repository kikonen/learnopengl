#include "Frame.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "render/RenderContext.h"

Frame::Frame(std::shared_ptr<Window> window)
    : m_window(window)
{
}

Frame::~Frame()
{
}

void Frame::prepare(const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;
}

void Frame::bind(const render::RenderContext& ctx)
{
    // feed inputs to dear imgui, start new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Frame::render(const render::RenderContext& ctx)
{
    // Render dear imgui into screen
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Frame::trackImGuiState(
    debug::DebugContext& dbg)
{
    auto& input = *m_window->m_input;

    ImGuiIO& io = ImGui::GetIO();

    input.imGuiHasKeyboard = io.WantCaptureKeyboard;
    input.imGuiHasMouse = io.WantCaptureMouse;
}
