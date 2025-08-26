#include "Frame.h"

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
    debug::DebugContext& dbg)
{
    auto& input = *m_window->m_input;

    input.imGuiHasKeyboard =
        ImGui::IsAnyItemActive() ||
        ImGui::IsAnyItemFocused() ||
        ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);

    input.imGuiHasMouse =
        ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
        ImGui::IsAnyItemHovered() ||
        ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
}
