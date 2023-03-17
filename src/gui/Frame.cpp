#include "Frame.h"

#include "render/RenderContext.h"

Frame::Frame(Window& window)
    : m_window(window)
{
}

Frame::~Frame()
{
}

void Frame::prepare()
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
