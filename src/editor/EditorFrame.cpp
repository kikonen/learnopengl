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
        trackImGuiState(debugContext);
        ImGui::End();
        return;
    }

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    //if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("learnopengl_editor");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    //}

    //if (ImGui::CollapsingHeader("Status"))
    {
        renderStatus(ctx, debugContext);
    }

    if (ImGui::CollapsingHeader("Node"))
    {
        renderNodeSelector(ctx, debugContext);
        if (ImGui::TreeNode("Edit")) {
            renderNodeEdit(ctx, debugContext);
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Debug")) {
            renderNodeDebug(ctx, debugContext);
            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Animation"))
    {
        renderAnimationDebug(ctx, debugContext);
    }

    if (ImGui::CollapsingHeader("Misc"))
    {
        renderMiscDebug(ctx, debugContext);
    }

    trackImGuiState(debugContext);

    ImGui::End();
}

void EditorFrame::trackImGuiState(
    render::DebugContext& debugContext)
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
    if (ImGui::BeginCombo("NodeSelector", currType ? currType->m_name.c_str() : nullptr)) {
        for (const auto* node : nr.getCachedNodesRT())
        {
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
    ImGui::Checkbox("Node debug", &debugContext.m_nodeDebugEnabled);

    if (debugContext.m_nodeDebugEnabled) {
        ImGui::Checkbox("Wireframe", &debugContext.m_wireframe);
        ImGui::Checkbox("Show normals", &debugContext.m_showNormals);
        ImGui::DragFloat3("Selection Axis", glm::value_ptr(debugContext.m_selectionAxis), -1.f, 1.f);
    }
}

void EditorFrame::renderAnimationDebug(
    const RenderContext& ctx,
    render::DebugContext& debugContext)
{
    const auto& assets = ctx.m_assets;

    ImGui::Checkbox("Animation debug", &debugContext.m_animationDebugEnabled);

    if (debugContext.m_animationDebugEnabled) {
        ImGui::Checkbox("Pause", &debugContext.m_animationPaused);
        ImGui::InputInt("Clip", &debugContext.m_animationClipIndex, 1, 10);

        if (assets.glslUseDebug) {
            ImGui::Checkbox("Bone debug", &debugContext.m_animationDebugBoneWeight);
            ImGui::InputInt("Bone index", &debugContext.m_animationBoneIndex, 1, 10);
        }
    }
}

void EditorFrame::renderMiscDebug(
    const RenderContext & ctx,
    render::DebugContext & debugContext)
{
    ImGui::Checkbox("ImGui Demo", &debugContext.m_imguiDemo);
    ImGui::Checkbox("Frustum enabled", &debugContext.m_frustumEnabled);
}
