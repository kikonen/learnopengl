#include "NodeTool.h"

#include <math.h>

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/util.h"
#include "asset/Assets.h"

#include "gui/Input.h"

#include "engine/Engine.h"

#include "event/Event.h"
#include "event/Dispatcher.h"

#include "script/CommandEngine.h"
#include "script/command/SelectNode.h"
#include "script/command/AudioPlay.h"
#include "script/command/AudioPause.h"

#include "model/NodeType.h"
#include "model/Node.h"
#include "model/CreateState.h"
#include "model/CompositeBuilder.h"

#include "render/NodeDraw.h"
#include "render/FrameBuffer.h"

#include "animation/RigContainer.h"
#include "animation/RigSocket.h"

#include "mesh/LodMesh.h"
#include "mesh/ModelMesh.h"

#include "render/RenderContext.h"

#include "scene/Scene.h"

#include "controller/PawnController.h"
#include "controller/CameraZoomController.h"

#include "registry/NodeRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/SelectionRegistry.h"

#include "editor/EditorFrame.h"

class PawnController;

namespace {
    const char* GEOMETRY_TYPES[2] = {
        "",
        "wireframe_mod",
    };
}

namespace editor
{
    NodeTool::NodeTool(EditorFrame& editor)
        : Tool{ editor, "Node" }
    { }

    NodeTool::~NodeTool() = default;

    void NodeTool::prepare(const PrepareContext& ctx)
    {
        auto* dispatcherView = ctx.m_registry->m_dispatcherView;

        dispatcherView->addListener(
            event::Type::node_select,
            [this](const event::Event& e) {
                const auto& data = e.body.select;
                if (auto nodeHandle = pool::NodeHandle::toHandle(data.target)) {
                    if (data.select) {
                        m_state.m_selectedNode = nodeHandle;
                    }
                }
            });
    }

    void NodeTool::drawImpl(
        const RenderContext& ctx,
        Scene* scene,
        render::DebugContext& dbg)
    {
        if (ImGui::CollapsingHeader("Node"))
        {
            renderNode(ctx, dbg);
        }

        if (ImGui::CollapsingHeader("Animation"))
        {
            renderAnimationDebug(ctx, dbg);
        }
    }

    void NodeTool::renderNode(
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

    void NodeTool::renderNodeSelector(
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

                if (ImGui::Selectable(name, node == currNode)) {
                    onSelectNode(ctx, node->toHandle());
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }
    }

    void NodeTool::renderNodeProperties(
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        auto* node = m_state.m_selectedNode.toNode();
        if (!node) return;

        auto& state = node->modifyState();

        {
            glm::vec3 pos = state.getPosition();
            // , "%.3f", ImGuiInputTextFlags_EnterReturnsTrue
            if (ImGui::InputFloat3("Position", glm::value_ptr(pos))) {
                state.setPosition(pos);
            }
        }

        {
            glm::vec3 rot = state.getDegreesRotation();
            // , "%.3f", ImGuiInputTextFlags_EnterReturnsTrue
            if (ImGui::InputFloat3("Rotation", glm::value_ptr(rot))) {
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
            if (ImGui::InputFloat3("Scale", glm::value_ptr(scale))) {
                state.setScale(scale);
            }
        }

        if (ImGui::Button("Delete"))
        {
            onDeleteNode(ctx, m_state.m_selectedNode);
        }

        ImGui::SameLine();

        if (ImGui::Button("Clone"))
        {
            onCloneNode(ctx, m_state.m_selectedNode);
        }
    }

    void NodeTool::renderTypeProperties(
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
                for (auto& lodMesh : node->getLodMeshes()) {
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

    void NodeTool::renderRigProperties(
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

    void NodeTool::renderNodeDebug(
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

    void NodeTool::renderAnimationDebug(
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

    void NodeTool::processInputs(
        const RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState)
    {
        if (inputState.mouseLeft != lastInputState.mouseLeft &&
            inputState.mouseLeft == GLFW_PRESS &&
            input.allowMouse())
        {
            if (inputState.ctrl)
            {
                handleSelectNode(ctx, scene, input, inputState, lastInputState);
            }
            //else if (inputState.shift)
            //{
            //    shoot(ctx, scene, input, inputState, m_lastInputState);
            //}
        }
    }

    void NodeTool::handleSelectNode(
        const RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState)
    {
        auto& window = m_editor.getWindow();

        const auto& assets = ctx.m_assets;
        auto& nodeRegistry = *ctx.m_registry->m_nodeRegistry;
        auto& selectionRegistry = *ctx.m_registry->m_selectionRegistry;
        auto& commandEngine = script::CommandEngine::get();

        auto& dbg = render::DebugContext::modify();

        const bool selectMode = inputState.ctrl;

        ki::node_id nodeId = scene->getObjectID(
            ctx,
            input.mouseX,
            input.mouseY);

        auto* node = pool::NodeHandle::toNode(nodeId);

        if (selectMode) {
            // deselect
            if (node) {
                const auto& handle = node->toHandle();

                if (selectionRegistry.isSelected(handle)) {
                    commandEngine.addCommand(
                        0,
                        script::SelectNode{
                            handle,
                            false,
                            false
                        });

                    commandEngine.addCommand(
                        0,
                        script::AudioPause{
                            handle,
                            SID("select")
                        });

                    m_state.m_selectedNode = 0;
                    return;
                }
                else {
                    commandEngine.addCommand(
                        0,
                        script::SelectNode{
                            handle,
                            true,
                            inputState.shift
                        });

                    commandEngine.addCommand(
                        0,
                        script::AudioPlay{
                            handle,
                            SID("select"),
                            false
                        });

                    m_state.m_selectedNode = handle;
                }
            }
            else {
                selectionRegistry.clearSelection();
                m_state.m_selectedNode = 0;
            }
        }

        //else if (playerMode) {
        //    if (node && inputState.ctrl) {
        //        auto exists = ControllerRegistry::get().hasController(node);
        //        if (exists) {
        //            event::Event evt { event::Type::node_activate };
        //            evt.body.node.target = node->getId();
        //            ctx.m_registry->m_dispatcherWorker->send(evt);
        //        }

        //        node = nullptr;
        //    }
        //}
        //else if (cameraMode) {
        //    // NOTE KI null == default camera
        //    event::Event evt { event::Type::camera_activate };
        //    evt.body.node.target = node->getId();
        //    ctx.m_registry->m_dispatcherView->send(evt);

        //    node = nullptr;
        //}
    }

    void NodeTool::onSelectNode(
        const RenderContext& ctx,
        pool::NodeHandle nodeHandle)
    {
        m_state.m_selectedNode = nodeHandle;

        auto& commandEngine = script::CommandEngine::get();
        commandEngine.addCommand(
            0,
            script::SelectNode{
                m_state.m_selectedNode,
                true,
                false
            });
    }

    void NodeTool::onDeleteNode(
        const RenderContext& ctx,
        pool::NodeHandle nodeHandle)
    {
        auto* node = m_state.m_selectedNode.toNode();
        if (!node) return;

        event::Event evt{ event::Type::node_remove };
        auto& body = evt.body.node = {
            .target = m_state.m_selectedNode.toId(),
        };
        ctx.m_registry->m_dispatcherWorker->send(evt);
    }

    void NodeTool::onCloneNode(
        const RenderContext& ctx,
        pool::NodeHandle nodeHandle)
    {
        auto* node = m_state.m_selectedNode.toNode();
        if (!node) return;

        const auto& state = node->getState();
        const auto* type = node->getType();

        {
            ki::node_id parentId{ node->getParentHandle().toId() };
            glm::vec3 pos{ state.getPosition() };
            glm::quat quat{ state.getRotation() };
            glm::vec3 scale{ state.getScale() };

            CreateState state{
                pos,
                scale,
                quat };

            CompositeBuilder builder{ NodeRegistry::get() };
            if (builder.build(parentId, 0, type, state)) {
                auto rootHandle = builder.asyncAttach(ctx.m_registry);

                auto& commandEngine = script::CommandEngine::get();
                commandEngine.addCommand(
                    0,
                    script::SelectNode{
                        rootHandle,
                        true,
                        false
                    });
            }
        }

    }

}
