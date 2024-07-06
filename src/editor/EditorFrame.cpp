#include "EditorFrame.h"

#include <math.h>

#include <glm/gtc/type_ptr.hpp>

#include "asset/Assets.h"

#include "engine/Engine.h"

#include "model/Node.h"

#include "mesh/MeshType.h"
#include "mesh/LodMesh.h"
#include "mesh/ModelMesh.h"

#include "render/RenderContext.h"

#include "registry/NodeRegistry.h"

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
    auto& debugContext = m_window.getEngine().m_debugContext;

    // NOTE KI don't waste CPU if Edit window is collapsed
    if (!ImGui::Begin("Edit")) {
        ImGui::End();
        return;
    }

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    //if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("learnopengl");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    //}

    //if (ImGui::CollapsingHeader("Status"))
    {
        renderStatus(ctx, debugContext);
    }

    if (ImGui::CollapsingHeader("Node"))
    {
        renderNodeSelector(ctx, debugContext);
        renderNodeEdit(ctx, debugContext);
        renderNodeDebug(ctx, debugContext);
    }

    if (ImGui::CollapsingHeader("Animation"))
    {
        renderBoneDebug(ctx, debugContext);
    }

    trackImGuiState(debugContext);

    ImGui::End();
}

void EditorFrame::trackImGuiState(
    render::DebugContext& debugContext)
{
    auto imguiHit = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
        ImGui::IsAnyItemHovered() ||
        ImGui::IsAnyItemActive() ||
        ImGui::IsAnyItemFocused();
    m_window.m_input->imGuiHit = imguiHit;
}

void EditorFrame::renderStatus(
    const RenderContext& ctx,
    render::DebugContext& debugContext)
{
    const auto& fpsCounter = m_window.getEngine().getFpsCounter();
    //auto fpsText = fmt::format("{} fps", round(fpsCounter.getAvgFps()));
    auto fpsSummary = fpsCounter.formatSummary("");
    ImGui::Text(fpsSummary.c_str());
}

void EditorFrame::renderNodeSelector(
    const RenderContext& ctx,
    render::DebugContext& debugContext)
{
    const auto& nr = NodeRegistry::get();

    const auto curr = debugContext.m_targetNode.toNode();
    const auto* currType = curr ? curr->m_typeHandle.toType() : nullptr;
    if (ImGui::BeginCombo("Node", currType ? currType->m_name.c_str() : nullptr)) {
        const auto& nodes = nr.getCachedNodesRT();
        for (int n = 0; n < nodes.size(); n++)
        {
            const auto* node = nodes[n];
            if (!node) continue;

            const auto* type = node->m_typeHandle.toType();
            const auto* name = type->m_name.c_str();

            ImGui::PushID((void*)node);
            if (ImGui::Selectable(name, node == curr))
                debugContext.m_targetNode = node->toHandle();
            ImGui::PopID();
        }

        ImGui::EndCombo();
    }
}

void EditorFrame::renderNodeEdit(
    const RenderContext& ctx,
    render::DebugContext& debugContext)
{
    static float rotation = 0.0;
    ImGui::DragFloat("rotation", &rotation, 0, 2);
    static float translation[] = { 0.f, 0.f, 0.f };
    ImGui::DragFloat3("position", translation, -1.0, 1.0);
    static float color[4] = { 1.0f,1.0f,1.0f,1.0f };
    // pass the parameters to the shader
    //    triangle_shader.setUniform("rotation", rotation);
    //    triangle_shader.setUniform("translation", translation[0], translation[1]);
        // color picker
    ImGui::ColorEdit3("color", color);
    // multiply triangle's color with this color
    //triangle_shader.setUniform("color", color[0], color[1], color[2]);
}

void EditorFrame::renderNodeDebug(
    const RenderContext& ctx,
    render::DebugContext& debugContext)
{
    ImGui::DragFloat3("Selection Axis", glm::value_ptr(debugContext.m_selectionAxis), -1.f, 1.f);
}

void EditorFrame::renderBoneDebug(
    const RenderContext& ctx,
    render::DebugContext& debugContext)
{
    const auto& assets = ctx.m_assets;

    if (assets.glslUseDebug) {
        ImGui::Checkbox("Bone debug", &debugContext.m_debugBoneWeight);
        ImGui::InputInt("Bone index", &debugContext.m_boneIndex, 1, 10);
    }
}
