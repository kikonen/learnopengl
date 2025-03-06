#include "Frame.h"

#include "render/RenderContext.h"

Frame::Frame(Window& window)
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

void Frame::trackImGuiState(
    render::DebugContext& dbg)
{
    m_window.m_input->imGuiHasKeyboard =
        ImGui::IsAnyItemActive() ||
        ImGui::IsAnyItemFocused() ||
        ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);

    m_window.m_input->imGuiHasMouse =
        ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
        ImGui::IsAnyItemHovered() ||
        ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
}
