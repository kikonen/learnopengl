#include "EditorFrame.h"

#include <math.h>

#include <glm/gtc/type_ptr.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/util.h"
#include "asset/Assets.h"
#include "asset/DynamicCubeMap.h"

#include "engine/Engine.h"

#include "event/Event.h"
#include "event/Dispatcher.h"

#include "model/Node.h"

#include "render/NodeDraw.h"
#include "render/FrameBuffer.h"

#include "animation/RigContainer.h"
#include "animation/RigSocket.h"

#include "mesh/MeshType.h"
#include "mesh/LodMesh.h"
#include "mesh/ModelMesh.h"

#include "decal/DecalRegistry.h"

#include "render/RenderContext.h"

#include "controller/PawnController.h"
#include "controller/CameraZoomController.h"

#include "model/Viewport.h"

#include "renderer/ObjectIdRenderer.h"
#include "renderer/WaterMapRenderer.h"
#include "renderer/MirrorMapRenderer.h"
#include "renderer/CubeMapRenderer.h"
#include "renderer/ShadowMapRenderer.h"

#include "registry/NodeRegistry.h"
#include "registry/ControllerRegistry.h"

#include "render/FrameBuffer.h"
#include "scene/Scene.h"

#include "console/ConsoleFrame.h"


class PawnController;

namespace {
    const char* GEOMETRY_TYPES[2] = {
        "",
        "wireframe_mod",
    };

    const std::vector<std::pair<ViewportEffect, std::string>> g_viewportEffects = {
        { ViewportEffect::none, "none"},
        { ViewportEffect::invert, "invert"},
        { ViewportEffect::gray_scale, "gray_scale"},
        { ViewportEffect::sharpen, "sharpen"},
        { ViewportEffect::blur, "blur"},
        { ViewportEffect::edge, "edge"},
    };
}

namespace editor {
    EditorFrame::EditorFrame(Window& window)
        : Frame(window),
        m_console{ std::make_unique<ConsoleFrame>(window) }
    {
    }

    EditorFrame::~EditorFrame() = default;

    void EditorFrame::prepare(const PrepareContext& ctx)
    {
        const auto& assets = ctx.m_assets;

        Frame::prepare(ctx);

        m_console->prepare(ctx);
    }

    void EditorFrame::draw(const RenderContext& ctx)
    {
        const auto& assets = ctx.m_assets;
        auto& dbg = m_window.getEngine().m_dbg;

        ctx.m_state.bindFrameBuffer(0, false);

        if (assets.imGuiDemo|| getState().m_showImguiDemo) {
            ImGui::ShowDemoWindow();
        }

        if (getState().m_showConsole) {
            m_console->draw(ctx);
        }

        //ImGuiIO& io = ImGui::GetIO();
        //io.ConfigFlags |= 0
        //    //| ImGuiConfigFlags_NavEnableKeyboard
        //    | 0;

        // NOTE KI don't waste CPU if Edit window is collapsed
        bool* openPtr = nullptr;
        ImGuiWindowFlags flags = 0
            | ImGuiWindowFlags_MenuBar
            | 0;
        if (!ImGui::Begin("Edit", openPtr, flags)) {
            trackImGuiState(dbg);
            ImGui::End();
            return;
        }

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        //if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("learnopengl_editor");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        //}

        renderMenuBar(ctx, dbg);

        //if (ImGui::CollapsingHeader("Status"))
        {
            renderStatus(ctx, dbg);
        }

        if (ImGui::CollapsingHeader("Camera"))
        {
            renderCameraDebug(ctx, dbg);
        }

        if (ImGui::CollapsingHeader("Node"))
        {
            renderNodeEdit(ctx, dbg);
        }

        if (ImGui::CollapsingHeader("Animation"))
        {
            renderAnimationDebug(ctx, dbg);
        }

        if (ImGui::CollapsingHeader("Viewport"))
        {
            renderBufferDebug(ctx, dbg);
        }

        if (ImGui::CollapsingHeader("Physics"))
        {
            renderPhysicsDebug(ctx, dbg);
        }

        if (ImGui::CollapsingHeader("Effect"))
        {
            renderEffectDebug(ctx, dbg);
        }

        if (ImGui::CollapsingHeader("Layers"))
        {
            renderLayersDebug(ctx, dbg);
        }

        if (ImGui::CollapsingHeader("Misc"))
        {
            renderMiscDebug(ctx, dbg);
        }

        trackImGuiState(dbg);

        ImGui::End();
    }

    void EditorFrame::renderMenuBar(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ImGui::Checkbox("ImGui Demo", &m_state.m_showImguiDemo);
                ImGui::Checkbox("Console", &m_state.m_showConsole);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    void EditorFrame::renderStatus(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        const auto& fpsCounter = m_window.getEngine().getFpsCounter();
        //auto fpsText = fmt::format("{} fps", round(fpsCounter.getAvgFps()));
        auto fpsSummary = fpsCounter.formatSummary("");
        ImGui::Text(fpsSummary.c_str());
    }

    void EditorFrame::renderCameraDebug(
        const RenderContext& ctx,
        render::DebugContext& dbg)
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

    void EditorFrame::renderNodeEdit(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        renderNodeSelector(ctx, dbg);

        {
            ImGui::Spacing();
            ImGui::Separator();

            renderNodeProperties(ctx, dbg);
            renderTypeProperties(ctx, dbg);
            renderRigProperties(ctx, dbg);
        }

        {
            ImGui::Spacing();
            ImGui::Separator();

            renderNodeDebug(ctx, dbg);
        }
    }

    void EditorFrame::renderNodeSelector(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        const auto& nr = NodeRegistry::get();

        const auto currNode = m_state.m_selectedNode.toNode();
        if (ImGui::BeginCombo("Node selector", currNode ? currNode->getName().c_str() : nullptr)) {
            for (const auto* node : nr.getCachedNodesRT())
            {
                if (!node) continue;

                const auto* name = node->getName().c_str();

                ImGui::PushID((void*)node);
                if (ImGui::Selectable(name, node == currNode))
                    m_state.m_selectedNode = node->toHandle();
                ImGui::PopID();
            }

            ImGui::EndCombo();
        }
    }

    void EditorFrame::renderNodeProperties(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        auto* node = m_state.m_selectedNode.toNode();
        if (!node) return;

        auto& state = node->modifyState();

        {
            glm::vec3 pos = state.getPosition();
            // , "%.3f", ImGuiInputTextFlags_EnterReturnsTrue
            if (ImGui::InputFloat3("Node position", glm::value_ptr(pos))) {
                state.setPosition(pos);
            }
        }

        {
            glm::vec3 rot = state.getDegreesRotation();
            // , "%.3f", ImGuiInputTextFlags_EnterReturnsTrue
            if (ImGui::InputFloat3("Node rotation", glm::value_ptr(rot))) {
                state.setDegreesRotation(rot);

                auto quat = util::degreesToQuat(rot);
                auto deg = util::quatToDegrees(quat);

                KI_INFO_OUT(fmt::format(
                    "rot={}, deg={}, quat={}",
                    rot, deg, quat));
            }
        }

        {
            glm::vec3 scale = state.getScale();
            if (ImGui::InputFloat3("Node scale", glm::value_ptr(scale))) {
                state.setScale(scale);
            }
        }
    }

    void EditorFrame::renderTypeProperties(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        auto* node = m_state.m_selectedNode.toNode();
        if (!node) return;

        {
            ImGui::SeparatorText("Mesh type");
            auto* type = node->m_typeHandle.toType();
            ImGui::Text(type->getName().c_str());

            const auto currMesh = m_state.m_selectedMesh;
            if (ImGui::BeginCombo("Mesh", currMesh ? currMesh->m_name.c_str() : nullptr)) {
                for (auto& lodMesh : type->getLodMeshes()) {
                    auto* mesh = lodMesh.getMesh<mesh::Mesh>();

                    const auto* name = mesh->m_name.c_str();

                    ImGui::PushID((void*)mesh);
                    const bool isSelected = currMesh == mesh;
                    if (ImGui::Selectable(name, isSelected)) {
                        m_state.m_selectedMesh = mesh;
                    }
                    ImGui::PopID();

                    //if (isSelected)
                    //    ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }
    }

    void EditorFrame::renderRigProperties(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        if (!m_state.m_selectedMesh) return;

        auto* mesh = m_state.m_selectedMesh;

        auto rig = mesh->getRigContainer();
        if (!rig) return;

        if (ImGui::CollapsingHeader("Rig"))
        {
            auto& clipContainer = rig->m_clipContainer;

            {
                auto* currSocket = rig->getSocket(m_state.m_selectedSocketIndex);
                if (ImGui::BeginCombo("Socket", currSocket ? currSocket->m_name.c_str() : nullptr)) {
                    for (auto& socket : rig->m_sockets) {
                        const auto* name = socket.m_name.c_str();

                        ImGui::PushID((void*)socket.m_index);
                        const bool isSelected = m_state.m_selectedSocketIndex == socket.m_index;
                        if (ImGui::Selectable(name, isSelected)) {
                            m_state.m_selectedSocketIndex = socket.m_index;
                        }
                        ImGui::PopID();

                        //if (isSelected)
                        //    ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }

            if (auto* socket = rig->modifySocket(m_state.m_selectedSocketIndex);  socket) {
                ImGui::Text(socket->m_jointName.c_str());

                {
                    glm::vec3 offset = socket->m_offset;
                    if (ImGui::InputFloat3("Socket offset", glm::value_ptr(offset))) {
                        socket->m_offset = offset;
                        socket->updateTransforms();
                    }
                }

                {
                    glm::vec3 rot = util::quatToDegrees(socket->m_rotation);
                    if (ImGui::InputFloat3("Socket rotation", glm::value_ptr(rot))) {
                        socket->m_rotation = util::degreesToQuat(rot);
                        socket->updateTransforms();
                    }
                }

                {
                    float scale = socket->m_scale;
                    if (ImGui::InputFloat("Socket scale", &scale)) {
                        socket->m_scale = scale;
                        socket->updateTransforms();
                    }
                }
            }

            if (!clipContainer.m_animations.empty())
            {
                auto* currAnim = clipContainer.getAnimation(m_state.m_selectedAnimationIndex);
                if (ImGui::BeginCombo("Animation", currAnim ? currAnim->m_name.c_str() : nullptr)) {
                    for (auto& anim : clipContainer.m_animations) {
                        const auto* name = anim->m_name.c_str();

                        const bool isSelected = m_state.m_selectedAnimationIndex == anim->getIndex();

                        ImGui::PushID((void*)anim->getIndex());
                        if (ImGui::Selectable(name, isSelected)) {
                            m_state.m_selectedAnimationIndex = anim->getIndex();
                        }
                        ImGui::PopID();

                        //if (isSelected)
                        //    ImGui::SetItemDefaultFocus();

                    }
                    ImGui::EndCombo();
                }
            }

            if (!clipContainer.m_clips.empty())
            {
                auto* currClip = clipContainer.getClip(m_state.m_selectedClipIndex);
                if (ImGui::BeginCombo("Clip", currClip ? currClip->m_name.c_str() : nullptr)) {
                    for (auto& clip : clipContainer.m_clips) {
                        const auto* name = clip.m_name.c_str();

                        ImGui::PushID((void*)clip.m_index);
                        const bool isSelected = m_state.m_selectedClipIndex == clip.m_index;
                        if (ImGui::Selectable(name, isSelected)) {
                            m_state.m_selectedClipIndex = clip.m_index;
                        }
                        ImGui::PopID();

                        //if (isSelected)
                        //    ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }
        }
    }

    void EditorFrame::renderNodeDebug(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        ImGui::Checkbox("Node debug", &dbg.m_nodeDebugEnabled);

        if (dbg.m_nodeDebugEnabled) {
            {
                if (ImGui::BeginCombo("GeomMod", dbg.m_geometryType.c_str())) {
                    for (const auto& name : GEOMETRY_TYPES) {

                        ImGui::PushID((void*)name);
                        const bool isSelected = dbg.m_geometryType == name;
                        if (ImGui::Selectable(name, isSelected)) {
                            dbg.m_geometryType = name;
                        }
                        ImGui::PopID();

                        //if (isSelected)
                        //    ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                if (dbg.m_geometryType == "wireframe_mod")
                {
                    ImGui::Spacing();
                    ImGui::SeparatorText("wireframe_mod");

                    //ImGui::Checkbox("WireframeMode", &dbg.m_wireframeMode);
                    ImGui::DragFloat("WireframeWidth", &dbg.m_wireframeLineWidth, 0.f, 1.f);
                    ImGui::ColorEdit3("WireframeColor", glm::value_ptr(dbg.m_wireframeLineColor));
                    ImGui::Checkbox("WireframeOnly", &dbg.m_wireframeOnly);
                }
            }
        }

        {
            ImGui::Spacing();
            ImGui::Separator();

            ImGui::Checkbox("Show normals", &dbg.m_showNormals);
            ImGui::DragFloat3("Selection Axis", glm::value_ptr(dbg.m_selectionAxis), -1.f, 1.f);
        }
    }

    void EditorFrame::renderAnimationDebug(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        const auto& assets = ctx.m_assets;

        ImGui::Checkbox("Pause", &dbg.m_animationPaused);
        ImGui::Checkbox("Animation debug", &dbg.m_animationDebugEnabled);

        if (dbg.m_animationDebugEnabled) {
            ImGui::SeparatorText("Animation blending");

            ImGui::Checkbox("Force first frame", &dbg.m_animationForceFirstFrame);
            if (!dbg.m_animationForceFirstFrame) {
                ImGui::Checkbox("Manual time", &dbg.m_animationManualTime);
                if (dbg.m_animationManualTime) {
                    ImGui::InputFloat("Current time", &dbg.m_animationCurrentTime, 0.01f, 0.1f);
                }
            }

            ImGui::SeparatorText("Clip A");
            ImGui::InputInt("Clip A", &dbg.m_animationClipIndexA, 1, 10);
            ImGui::InputFloat("Clip A start", &dbg.m_animationStartTimeA, 0.01f, 0.1f);
            ImGui::InputFloat("Clip A speed", &dbg.m_animationSpeedA, 0.01f, 0.1f);

            ImGui::SeparatorText("Clip B");
            ImGui::Checkbox("Blend animation", &dbg.m_animationBlend);

            if (dbg.m_animationBlend) {
                ImGui::InputFloat("Blend factor", &dbg.m_animationBlendFactor, 0.01f, 0.1f);

                ImGui::InputInt("Clip B", &dbg.m_animationClipIndexB, 1, 10);
                ImGui::InputFloat("Clip B start", &dbg.m_animationStartTimeB, 0.01f, 0.1f);
                ImGui::InputFloat("Clip B speed", &dbg.m_animationSpeedB, 0.01f, 0.1f);
            }

            if (assets.glslUseDebug) {
                ImGui::SeparatorText("Bone visualization");

                ImGui::Checkbox("Bone debug", &dbg.m_animationDebugBoneWeight);
                ImGui::InputInt("Bone index", &dbg.m_animationBoneIndex, 1, 10);
            }
        }
    }

    void EditorFrame::renderBufferDebug(
        const RenderContext& ctx,
        render::DebugContext& dbg)
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
            ImTextureID texId = att.textureID;
            ImGui::Image(
                texId,
                ImVec2{ w, h },
                ImVec2{ 0, 1 }, // uv1
                ImVec2{ 1, 0 }, // uv2
                ImVec4{ 1, 1, 1, 1 }, // tint_col
                ImVec4{ 1, 1, 1, 1 }  // border_col
            );
            };

        auto bufferTex = [&ctx](render::FrameBuffer& fb, int attachmentIndex, bool useAspectRatio) {
            ImVec2 availSize = ImGui::GetContentRegionAvail();
            // NOTE KI allow max half window size
            float w = std::min(availSize.x, ctx.m_resolution.x / 2.f) - scrollbarPadding;
            float h = w / ctx.m_aspectRatio;
            if (!useAspectRatio) {
                w = h;
            }

            // https://stackoverflow.com/questions/38543155/opengl-render-face-of-cube-map-to-a-quad

            const auto& att = fb.m_spec.attachments[attachmentIndex];
            ImTextureID texId = att.textureID;
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

                ImTextureID texId = fb.getTextureID();
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

        {
            //const auto& fb = scene.m_nodeDraw->m_oitBuffer.m_buffer;
            //int bufferIndex = 0;
            //for (const auto& att : fb->m_spec.attachments) {
            //    if (att.drawBufferIndex < 0) continue;

            //    const auto& name = fmt::format("OIT: {} - {}", bufferIndex, att.name);
            //    if (ImGui::TreeNodeEx(name.c_str(), tnFlags)) {
            //        bufferTex(*fb, att.index, true);
            //        ImGui::TreePop();
            //    }

            //    bufferIndex++;
            //}
        }
        {
            //const auto& fb = scene.m_nodeDraw->m_effectBuffer.m_primary;
            //int bufferIndex = 0;
            //for (const auto& att : fb->m_spec.attachments) {
            //    if (att.drawBufferIndex < 0) continue;

            //    const auto& name = fmt::format("Effect primary: {} - {}", bufferIndex, att.name);
            //    if (ImGui::TreeNodeEx(name.c_str(), tnFlags)) {
            //        bufferTex(*fb, att.index, true);
            //        ImGui::TreePop();
            //    }

            //    bufferIndex++;
            //}
        }
        {
            //const auto& fb = scene.m_nodeDraw->m_effectBuffer.m_secondary;
            //int bufferIndex = 0;
            //for (const auto& att : fb->m_spec.attachments) {
            //    if (att.drawBufferIndex < 0) continue;

            //    const auto& name = fmt::format("Effect secondary: {} - {}", bufferIndex, att.name);
            //    if (ImGui::TreeNodeEx(name.c_str(), tnFlags)) {
            //        bufferTex(*fb, att.index, true);
            //        ImGui::TreePop();
            //    }

            //    bufferIndex++;
            //}
        }
        {
            //int bufferIndex = 0;
            //for (const auto& fb : scene.m_nodeDraw->m_effectBuffer.m_buffers) {
            //    for (const auto& att : fb->m_spec.attachments) {
            //        if (att.drawBufferIndex < 0) continue;

            //        const auto& name = fmt::format("Effect buffers: {} - {}", bufferIndex, att.name);
            //        if (ImGui::TreeNodeEx(name.c_str(), tnFlags)) {
            //            bufferTex(*fb, att.index, true);
            //            ImGui::TreePop();
            //        }

            //        bufferIndex++;
            //    }
            //}
        }
        {
            //const auto& fb = scene.m_nodeDraw->m_gBuffer.m_buffer;
            //int bufferIndex = 0;
            //for (const auto& att : fb->m_spec.attachments) {
            //    if (att.drawBufferIndex < 0) continue;

            //    const auto& name = fmt::format("GBuffer: {} - {}", bufferIndex, att.name);
            //    if (ImGui::TreeNodeEx(name.c_str(), tnFlags)) {
            //        bufferTex(*fb, att.index, true);
            //        ImGui::TreePop();
            //    }

            //    bufferIndex++;
            //}
        }
    }

    void EditorFrame::renderPhysicsDebug(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        ImGui::Checkbox("Physics update enabled", &dbg.m_physicsUpdateEnabled);
        ImGui::Checkbox("Physics show objects", &dbg.m_physicsShowObjects);

        //ImGui::Checkbox("Physics show objects", &dbg.m_physicsShowObjects)
        //{
        //}
    }

    void EditorFrame::renderEffectDebug(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        {
            ImGui::Spacing();
            ImGui::SeparatorText("Gamma correction");
            ImGui::Spacing();

            ImGui::Checkbox("Gamma correct enabled", &dbg.m_gammaCorrectEnabled);
            ImGui::Checkbox("HW gamma correct enabled", &dbg.m_hardwareCorrectGammaEnabled);
            ImGui::InputFloat("SW Gamma", &dbg.m_gammaCorrect, 0.01f, 0.1f);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("HDR");
            ImGui::Spacing();

            ImGui::Checkbox("HDR tone mapping enabled", &dbg.m_hdrToneMappingEnabled);
            ImGui::InputFloat("HDR exposure", &dbg.m_hdrExposure, 0.01f, 0.1f);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Misc effects");
            ImGui::Spacing();

            ImGui::Checkbox("Prepass depth enabled", &dbg.m_prepassDepthEnabled);
            ImGui::Checkbox("OIT enabled", &dbg.m_effectOitEnabled);
            ImGui::Checkbox("Emission enabled", &dbg.m_effectEmissionEnabled);
            ImGui::Checkbox("Particle enabled", &dbg.m_particleEnabled);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Fog");
            ImGui::Spacing();

            ImGui::Checkbox("Fog enabled", &dbg.m_effectFogEnabled);
            ImGui::ColorEdit3("Fog color", glm::value_ptr(dbg.m_fogColor));
            ImGui::InputFloat("Fog start", &dbg.m_fogStart, 0.01f, 0.1f);
            ImGui::InputFloat("Fog end", &dbg.m_fogEnd, 0.01f, 0.1f);
            ImGui::InputFloat("Fog density", &dbg.m_fogDensity, 0.01f, 0.1f);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Cube map");
            ImGui::Spacing();

            ImGui::Checkbox("Cube map enabled", &dbg.m_cubeMapEnabled);
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Decals");
            ImGui::Spacing();

            ImGui::Checkbox("Decal enabled", &dbg.m_decalEnabled);

            {
                ImGui::Spacing();
                ImGui::Separator();

                if (ImGui::BeginCombo("Decal", dbg.m_decalId.str().c_str())) {
                    for (const auto& decalId : decal::DecalRegistry::get().getDecalIds()) {
                        const auto& name = decalId.str().c_str();

                        ImGui::PushID((void*)name);
                        const bool isSelected = dbg.m_decalId == decalId;
                        if (ImGui::Selectable(name, isSelected)) {
                            dbg.m_decalId = decalId;
                        }
                        ImGui::PopID();

                        //if (isSelected)
                        //    ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Bloom");
            ImGui::Spacing();

            ImGui::Checkbox("Bloom enabled", &dbg.m_effectBloomEnabled);
            ImGui::InputFloat("Bloom threshold", &dbg.m_effectBloomThreshold, 0.01f, 0.1f );
        }

        {
            ImGui::Spacing();
            ImGui::SeparatorText("Lighting");
            ImGui::Spacing();

            ImGui::Checkbox("Light enabled", &dbg.m_lightEnabled);
            ImGui::Checkbox("Normal map enabled", &dbg.m_normalMapEnabled);
        }
        {
            ImGui::Spacing();
            ImGui::SeparatorText("Parallax");
            ImGui::Spacing();

            ImGui::Checkbox("Parallax enabled", &dbg.m_parallaxEnabled);
            ImGui::InputInt("Parallax method", &dbg.m_parallaxMethod, 1, 10);
            ImGui::Checkbox("Parallax debug enabled", &dbg.m_parallaxDebugEnabled);
            ImGui::InputFloat("Parallax debug depth", &dbg.m_parallaxDebugDepth, 0.01f, 0.1f);
        }
    }

    void EditorFrame::renderLayersDebug(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        for (auto& layer : dbg.m_layers)
        {
            const auto idx = layer.m_index;

            if (idx == LAYER_NONE_INDEX) continue;

            ImGui::Spacing();
            ImGui::SeparatorText(fmt::format("Layer {}", layer.m_name).c_str());
            ImGui::Spacing();

            ImGui::Checkbox(
                fmt::format("L{}: enabled", idx).c_str(),
                &layer.m_enabled);

            ImGui::InputInt(
                fmt::format("L{}: Order", idx).c_str(),
                &layer.m_order, 0, 10);

            ImGui::Checkbox(
                fmt::format("L{}: Effect enabled", idx).c_str(),
                &layer.m_effectEnabled);

            {
                auto& curr = g_viewportEffects[util::as_integer(layer.m_effect)];

                if (ImGui::BeginCombo(
                    fmt::format("L{}: Effect", idx).c_str(),
                    curr.second.c_str()))
                {
                    for (const auto& [effect, name] : g_viewportEffects) {
                        ImGui::PushID((void*)effect);
                        if (ImGui::Selectable(name.c_str(), effect == curr.first)) {
                            layer.m_effect = effect;
                        }
                        ImGui::PopID();
                    }

                    ImGui::EndCombo();
                }
            }

            ImGui::Checkbox(
                fmt::format("L{}: Blend enabled", idx).c_str(),
                &layer.m_blendEnabled);

            ImGui::InputFloat(
                fmt::format("L{}: Blend factor", idx).c_str(),
                &layer.m_blendFactor, 0.01f, 0.1f);
        }
    }

    void EditorFrame::renderMiscDebug(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        {
            ImGui::Checkbox("Show volume", &dbg.m_showVolume);
            ImGui::Checkbox("Show selection volume", &dbg.m_showSelectionVolume);
            ImGui::Checkbox("Show environment probes", &dbg.m_showEnvironmentProbe);
        }

        {
            ImGui::InputInt("Show Font", &dbg.m_showFontId, 1, 10);
        }

        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::InputInt("Swap interval", &dbg.m_glfwSwapInterval, 1, 10);

            ImGui::InputFloat("GBuffer scale", &dbg.m_gBufferScale, 0.125f, 0.25f);

            ImGui::Checkbox("Frustum enabled", &dbg.m_frustumEnabled);
            ImGui::Checkbox("Draw debug", &dbg.m_drawDebug);

            ImGui::Checkbox("LineMode", &dbg.m_forceLineMode);
            ImGui::Checkbox("Shadow visual", &dbg.m_shadowVisual);
        }
    }
}
