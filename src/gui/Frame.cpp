#include "Frame.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "render/RenderContext.h"

#include "engine/Engine.h"

namespace gui
{
    Frame::Frame(const std::shared_ptr<Window>& window)
        : m_window(window)
    {
    }

    Frame::~Frame()
    {
    }

    Registry* Frame::getRegistry()
    {
        return m_window->getEngine().getRegistry();
    }

    void Frame::prepare(const PrepareContext& ctx)
    {
        if (m_prepared) return;
        m_prepared = true;
    }

    void Frame::bind(const gui::FrameContext& ctx)
    {
        // feed inputs to dear imgui, start new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Frame::render(const gui::FrameContext& ctx)
    {
        // Render dear imgui into screen
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Frame::trackImGuiState(
        const gui::FrameContext& ctx)
    {
        auto& input = *m_window->m_input;

        ImGuiIO& io = ImGui::GetIO();

        input.imGuiHasKeyboard = io.WantCaptureKeyboard;
        input.imGuiHasMouse = io.WantCaptureMouse;
    }
}
