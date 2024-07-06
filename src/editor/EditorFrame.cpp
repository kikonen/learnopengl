#include "EditorFrame.h"

#include <math.h>

#include <glm/gtc/type_ptr.hpp>

#include "asset/Assets.h"

#include "engine/Engine.h"

#include "render/RenderContext.h"

EditorFrame::EditorFrame(Window& window)
    : Frame(window)
{
}

void EditorFrame::prepare(const PrepareContext& ctx)
{
    const auto& assets = ctx.m_assets;

    Frame::prepare(ctx);
}

void EditorFrame::draw(const RenderContext& ctx)
{
    const auto& assets = Assets::get();
    auto& engine = m_window.getEngine();
    auto& debugContext = engine.m_debugContext;

    // render your GUI
    ImGui::Begin("Edit");

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    //if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("learnopengl");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        //}

    const auto& fpsCounter = m_window.getEngine().getFpsCounter();
    //auto fpsText = fmt::format("{} fps", round(fpsCounter.getAvgFps()));
    auto fpsSummary = fpsCounter.formatSummary("");
    ImGui::Text(fpsSummary.c_str());

    static float rotation = 0.0;
    ImGui::SliderFloat("rotation", &rotation, 0, 2);
    static float translation[] = { 0.0, 0.0 };
    ImGui::SliderFloat2("position", translation, -1.0, 1.0);
    static float color[4] = { 1.0f,1.0f,1.0f,1.0f };
    // pass the parameters to the shader
//    triangle_shader.setUniform("rotation", rotation);
//    triangle_shader.setUniform("translation", translation[0], translation[1]);
    // color picker
    ImGui::ColorEdit3("color", color);
    // multiply triangle's color with this color
    //triangle_shader.setUniform("color", color[0], color[1], color[2]);

    if (assets.glslUseDebug) {
        ImGui::Checkbox("bone debug", &debugContext.m_debugBoneWeight);
        ImGui::InputInt("bone index", &debugContext.m_boneIndex, 1, 10);
    }

    {
        ImGui::SliderFloat3("Selection Axis", glm::value_ptr(debugContext.m_selectionAxis), -1.f, 1.f);
    }

    {
        auto imguiHit = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
            ImGui::IsAnyItemHovered() ||
            ImGui::IsAnyItemActive() ||
            ImGui::IsAnyItemFocused();
        m_window.m_input->imGuiHit = imguiHit;
    }

    ImGui::End();
}

