#include "EditorFrame.h"

#include <math.h>

#include <glm/gtc/type_ptr.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "asset/Assets.h"
#include "asset/DynamicCubeMap.h"

#include "engine/Engine.h"

#include "event/Event.h"
#include "event/Dispatcher.h"

#include "model/Node.h"

#include "mesh/MeshType.h"
#include "mesh/LodMesh.h"
#include "mesh/ModelMesh.h"


#include "render/RenderContext.h"

#include "controller/PawnController.h"
#include "controller/CameraZoomController.h"

#include "model/Viewport.h"

#include "renderer/ObjectIdRenderer.h"
#include "renderer/NodeRenderer.h"
#include "renderer/WaterMapRenderer.h"
#include "renderer/MirrorMapRenderer.h"
#include "renderer/CubeMapRenderer.h"
#include "renderer/ShadowMapRenderer.h"

#include "registry/NodeRegistry.h"
#include "registry/NodeSnapshotRegistry.h"
#include "registry/ControllerRegistry.h"

#include "render/FrameBuffer.h"
#include "scene/Scene.h"

class PawnController;

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
    bool* openPtr = nullptr;
    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
    if (!ImGui::Begin("Edit", openPtr, flags)) {
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

    if (ImGui::CollapsingHeader("Camera"))
    {
        renderCameraDebug(ctx, debugContext);
    }

    if (ImGui::CollapsingHeader("Node"))
    {
        renderNodeSelector(ctx, debugContext);
        if (ImGui::TreeNode("Properties")) {
            renderNodeProperties(ctx, debugContext);
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

    if (ImGui::CollapsingHeader("Viewport"))
    {
        renderBufferDebug(ctx, debugContext);
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

void EditorFrame::renderCameraDebug(
    const RenderContext& ctx,
    render::DebugContext& debugContext)
{
    const auto& nr = NodeRegistry::get();
    const auto& cr = ControllerRegistry::get();

    // Pawn
    {
        const auto* currNode = nr.getActiveNode();
        if (ImGui::BeginCombo("Pawn selector", currNode ? currNode->getName().c_str() : nullptr)) {
            for (const auto& [nodeHandle, controllers] : cr.getControllers()) {
                const PawnController* nodeController = nullptr;
                for (const auto& controller : controllers) {
                    nodeController = dynamic_cast<const PawnController*>(&(*controller));
                    if (nodeController) break;
                }

                if (nodeController) {
                    const auto* node = nodeHandle.toNode();
                    if (!node) continue;

                    const auto* name = node->getName().c_str();

                    ImGui::PushID((void*)node);
                    if (ImGui::Selectable(name, node == currNode)) {
                        event::Event evt { event::Type::node_activate };
                        evt.body.node.target = node->getId();
                        ctx.m_registry->m_dispatcherWorker->send(evt);
                    }
                    ImGui::PopID();
                }
            }

            ImGui::EndCombo();
        }
    }

    // Camera
    {

        const auto* currNode = nr.getActiveCameraNode();
        if (ImGui::BeginCombo("Camera selector", currNode ? currNode->getName().c_str() : nullptr)) {
            for (const auto& [nodeHandle, controllers] : cr.getControllers()) {
                const CameraZoomController* nodeController = nullptr;
                for (const auto& controller : controllers) {
                    nodeController = dynamic_cast<const CameraZoomController*>(&(*controller));
                    if (nodeController) break;
                }

                if (nodeController) {
                    const auto* node = nodeHandle.toNode();
                    if (!node) continue;

                    const auto* name = node->getName().c_str();

                    ImGui::PushID((void*)node);
                    if (ImGui::Selectable(name, node == currNode)) {
                        event::Event evt { event::Type::camera_activate };
                        evt.body.node.target = node->getId();
                        ctx.m_registry->m_dispatcherWorker->send(evt);
                    }
                    ImGui::PopID();
                }
            }

            ImGui::EndCombo();
        }
    }
}

void EditorFrame::renderNodeSelector(
    const RenderContext& ctx,
    render::DebugContext& debugContext)
{
    const auto& nr = NodeRegistry::get();

    const auto currNode = debugContext.m_selectedNode.toNode();
    if (ImGui::BeginCombo("Node selector", currNode ? currNode->getName().c_str() : nullptr)) {
        for (const auto* node : nr.getCachedNodesRT())
        {
            if (!node) continue;

            const auto* name = node->getName().c_str();

            ImGui::PushID((void*)node);
            if (ImGui::Selectable(name, node == currNode))
                debugContext.m_selectedNode = node->toHandle();
            ImGui::PopID();
        }

        ImGui::EndCombo();
    }
}

void EditorFrame::renderNodeProperties(
    const RenderContext& ctx,
    render::DebugContext& debugContext)
{
    auto* node = debugContext.m_selectedNode.toNode();
    if (!node) return;

    auto& state = node->modifyState();

    {
        glm::vec3 pos = state.m_position;
        // , "%.3f", ImGuiInputTextFlags_EnterReturnsTrue
        if (ImGui::InputFloat3("position", glm::value_ptr(pos))) {
            state.setPosition(pos);
        }
    }

    {
        glm::vec3 rot = state.getDegreesRotation();
        // , "%.3f", ImGuiInputTextFlags_EnterReturnsTrue
        if (ImGui::InputFloat3("rotation", glm::value_ptr(rot))) {
            state.setDegreesRotation(rot);

            auto quat = util::degreesToQuat(rot);
            auto deg = util::quatToDegrees(quat);

            KI_INFO_OUT(fmt::format(
                "rot={}, deg={}, quat={}",
                rot, deg, quat));
        }
    }
}

void EditorFrame::renderNodeDebug(
    const RenderContext& ctx,
    render::DebugContext& debugContext)
{
    ImGui::Checkbox("Node debug", &debugContext.m_nodeDebugEnabled);

    if (debugContext.m_nodeDebugEnabled) {
        ImGui::Checkbox("Wireframe", &debugContext.m_forceWireframe);
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
        ImGui::InputFloat("Time", &debugContext.m_animationTime, 0.01f, 0.1f);

        if (assets.glslUseDebug) {
            ImGui::Checkbox("Bone debug", &debugContext.m_animationDebugBoneWeight);
            ImGui::InputInt("Bone index", &debugContext.m_animationBoneIndex, 1, 10);
        }
    }
}

void EditorFrame::renderBufferDebug(
    const RenderContext& ctx,
    render::DebugContext& debugContext)
{
    const auto& assets = ctx.m_assets;

    auto& window = m_window;
    auto& scene = *window.getEngine().getCurrentScene();

    constexpr float scrollbarPadding = 0.f;

    ImGuiTreeNodeFlags tnFlags = ImGuiTreeNodeFlags_SpanAvailWidth;

    auto viewportTex = [&ctx](Viewport& viewport, bool useAspectRatio) {
        ImVec2 availSize = ImGui::GetContentRegionAvail();
        // NOTE KI allow max half window size
        float w = std::min(availSize.x, ctx.m_resolution.x / 2.f) - scrollbarPadding;
        float h = w / ctx.m_aspectRatio;
        if (!useAspectRatio) {
            w = h;
        }

        viewport.invokeBindBefore();
        const auto& fb = viewport.getSourceFrameBuffer();
        auto& att = fb->m_spec.attachments[0];
        ImTextureID texId = (void*)att.textureID;
        ImGui::Image(
            texId,
            ImVec2{ w, h },
            ImVec2{ 0, 1 }, // uv1
            ImVec2{ 1, 0 }, // uv2
            ImVec4{ 1, 1, 1, 1 }, // tint_col
            ImVec4{ 1, 1, 1, 1 }  // border_col
        );
    };

    if (ImGui::TreeNodeEx("ObjectId", tnFlags)) {
        auto& viewport = scene.m_objectIdRenderer->m_debugViewport;
        viewportTex(*viewport, true);

        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Water reflection", tnFlags)) {
        auto& viewport = scene.m_waterMapRenderer->m_reflectionDebugViewport;
        viewportTex(*viewport, true);

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Water refraction", tnFlags)) {
        auto& viewport = scene.m_waterMapRenderer->m_refractionDebugViewport;
        viewportTex(*viewport, true);

        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Mirror reflection", tnFlags)) {
        auto& viewport = scene.m_mirrorMapRenderer->m_reflectionDebugViewport;
        viewportTex(*viewport, true);

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Mirror - Mirror reflection", tnFlags)) {
        auto& viewport = scene.m_mirrorMapRenderer->m_mirrorMapRenderer->m_reflectionDebugViewport;
        viewportTex(*viewport, true);

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Mirror - Water reflection", tnFlags)) {
        auto& viewport = scene.m_mirrorMapRenderer->m_waterMapRenderer->m_reflectionDebugViewport;
        viewportTex(*viewport, true);

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Mirror - Water refraction", tnFlags)) {
        auto& viewport = scene.m_mirrorMapRenderer->m_waterMapRenderer->m_refractionDebugViewport;
        viewportTex(*viewport, true); 

        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Cube - Mirror reflection", tnFlags)) {
        auto& viewport = scene.m_cubeMapRenderer->m_mirrorMapRenderer->m_reflectionDebugViewport;
        viewportTex(*viewport, false);

        ImGui::TreePop();
    }
    if (ImGui::TreeNodeEx("Cube - Water reflection", tnFlags)) {
        auto& viewport = scene.m_cubeMapRenderer->m_waterMapRenderer->m_reflectionDebugViewport;
        viewportTex(*viewport, false);

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Cube - Water refraction")) {
        auto& viewport = scene.m_cubeMapRenderer->m_waterMapRenderer->m_refractionDebugViewport;
        viewportTex(*viewport, false);

        ImGui::TreePop();
    }

    if (false && ImGui::TreeNodeEx("Cube - Faces", tnFlags)) {
        auto& cmr = *scene.m_cubeMapRenderer;

        auto faceTex = [&ctx, &cmr](int faceIndex) {
            ImVec2 availSize = ImGui::GetContentRegionAvail();
            // NOTE KI allow max half window size
            float w = std::min(availSize.x, ctx.m_resolution.x / 2.f) - scrollbarPadding;
            float h = w / ctx.m_aspectRatio;
            w = h;

            // https://stackoverflow.com/questions/38543155/opengl-render-face-of-cube-map-to-a-quad
            auto& prev = cmr.m_prev;
            auto fb = prev->asFrameBuffer(faceIndex);
            //fb.bind(ctx);
            //fb.bindFace();

            ImTextureID texId = (void*)fb.getTextureID();
            ImGui::Image(
                texId,
                ImVec2{ w, h },
                ImVec2{ 0, 1 }, // uv1
                ImVec2{ 1, 0 }, // uv2
                ImVec4{ 1, 1, 1, 1 }, // tint_col
                ImVec4{ 1, 1, 1, 1 }  // border_col
            );
        };

        for (unsigned int face = 0; face < 6; face++) {
            const auto& name = fmt::format("Cube - Face {}", face);
            if (ImGui::TreeNodeEx(name.c_str(), tnFlags)) {
                faceTex(face);

                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }

    if (assets.showRearView) {
        if (ImGui::TreeNodeEx("Rear view", tnFlags)) {
            auto& viewport = scene.m_rearViewport;
            viewportTex(*viewport, true);

            ImGui::TreePop();
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
