#include "NodeTool.h"

#include <unordered_map>

#include <math.h>

#include <imgui.h>

#include <glm/gtc/type_ptr.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/util.h"
#include "asset/Assets.h"

#include "engine/InputContext.h"
#include "engine/Engine.h"

#include "event/Event.h"
#include "event/Dispatcher.h"

#include "animation/AnimationSystem.h"
#include "animation/RigNodeRegistry.h"
#include "animation/JointRegistry.h"
#include "animation/SocketRegistry.h"

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

#include "NodeTree.h"

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
        auto* dispatcherView = ctx.getRegistry()->m_dispatcherView;

        m_listen_node_select.listen(
            event::Type::node_select,
            dispatcherView,
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
        const gui::FrameContext& ctx)
    {
        if (ImGui::CollapsingHeader("Node"))
        {
            ImGui::PushID("node");
            renderNode(ctx);
            ImGui::PopID();
        }

        if (ImGui::CollapsingHeader("Animation"))
        {
            ImGui::PushID("animation");
            renderAnimationDebug(ctx);
            ImGui::PopID();
        }
    }

    void NodeTool::renderNode(
        const gui::FrameContext& ctx)
    {
        renderNodeSelector(ctx);

        {
            ImGui::Spacing();
            ImGui::Separator();

            renderNodeProperties(ctx);
            renderTypeProperties(ctx);
            renderRigProperties(ctx);
        }

        {
            ImGui::Spacing();
            ImGui::Separator();

            renderNodeDebug(ctx);
        }
    }

    void NodeTool::renderNodeSelector(
        const gui::FrameContext& ctx)
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

        {
            const auto& nodes = NodeRegistry::get().getCachedNodesRT();
            NodeTree tree{ *this };
            tree.build(nodes);
            if (tree.m_root) {
                tree.drawNode(ctx, tree.m_root.get(), true);
            }
        }
    }

    void NodeTool::renderNodeProperties(
        const gui::FrameContext& ctx)
    {
        auto* node = m_state.m_selectedNode.toNode();
        if (!node) return;

        auto& state = node->modifyState();

        {
            glm::vec3 pivotAlignment = state.getPivotAlignment();
            // , "%.3f", ImGuiInputTextFlags_EnterReturnsTrue
            if (ImGui::InputFloat3("Pivot alignment", glm::value_ptr(pivotAlignment))) {
                state.setPivotAlignment(pivotAlignment);
            }
        }
        {
            glm::vec3 pivotOffset = state.getPivotOffset();
            // , "%.3f", ImGuiInputTextFlags_EnterReturnsTrue
            if (ImGui::InputFloat3("Pivot offset", glm::value_ptr(pivotOffset))) {
                state.setPivotOffset(pivotOffset);
            }
        }

        {
            glm::vec3 pos = state.getPosition();
            // , "%.3f", ImGuiInputTextFlags_EnterReturnsTrue
            if (ImGui::InputFloat3("Position", glm::value_ptr(pos))) {
                state.setPosition(pos);
            }
        }

        {
            auto& rotation = m_state.m_nodeRotation;
            rotation.update(state.getRotation());
            auto& rot = rotation.m_degrees;

            // , "%.3f", ImGuiInputTextFlags_EnterReturnsTrue
            if (ImGui::InputFloat3("Euler XYZ", glm::value_ptr(rot)))
            {
                state.setDegreesRotation(rot);
                rotation.mark(state.getRotation());
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
        const gui::FrameContext& ctx)
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
        const gui::FrameContext& ctx)
    {
        if (!m_state.m_selectedNode) return;
        if (!m_state.m_selectedMesh) return;

        auto* node = m_state.m_selectedNode.toNode();
        auto* mesh = m_state.m_selectedMesh;

        auto* rig = mesh->getRigContainer().get();
        if (!rig) return;

        if (ImGui::CollapsingHeader("Rig", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const auto& clipContainer = rig->getClipContainer();

            {
                auto* currSocket = rig->getSocket(m_state.m_selectedSocketIndex);
                if (ImGui::BeginCombo("Socket", currSocket ? currSocket->m_name.c_str() : nullptr)) {
                    for (const auto& socket : rig->getSockets()) {
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

                auto& offset = socket->m_offset;

                {
                    glm::vec3 pos = socket->m_offset.m_position;
                    if (ImGui::InputFloat3("Socket position", glm::value_ptr(pos))) {
                        socket->m_offset.m_position = pos;
                        updateSocket(node, mesh, rig, socket);
                    }
                }

                {
                    auto& rotation = m_state.m_socketRotation;
                    rotation.update(socket->m_offset.m_rotation);
                    auto& rot = rotation.m_degrees;

                    if (ImGui::InputFloat3("Socket rotation", glm::value_ptr(rot))) {
                        socket->m_offset.m_rotation = util::degreesToQuat(rot);
                        rotation.mark(socket->m_offset.m_rotation);

                        updateSocket(node, mesh, rig, socket);
                    }
                }

                {
                    float scale = socket->m_offset.m_scale.x;
                    if (ImGui::InputFloat("Socket scale", &scale)) {
                        socket->m_offset.setScale(scale);
                        updateSocket(node, mesh, rig, socket);
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
                if (ImGui::BeginCombo("Clip", currClip ? currClip->getName().c_str() : nullptr)) {
                    for (auto& clip : clipContainer.m_clips) {
                        const auto* name = clip.getName().c_str();

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
        const gui::FrameContext& ctx)
    {
        auto& dbg = ctx.getDebug().edit();

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
        const gui::FrameContext& ctx)
    {
        const auto& assets = Assets::get();
        auto& dbg = ctx.getDebug().edit();

        auto& anim = dbg.m_animation;

        ImGui::Checkbox("Pause", &anim.m_paused);
        ImGui::Checkbox("Animation debug", &anim.m_debugEnabled);

        if (anim.m_debugEnabled) {
            ImGui::SeparatorText("Animation blending");

            ImGui::Checkbox("Force first frame", &anim.m_forceFirstFrame);
            if (!anim.m_forceFirstFrame) {
                ImGui::Checkbox("Manual time", &anim.m_manualTime);
                if (anim.m_manualTime) {
                    ImGui::InputFloat("Current time", &anim.m_currentTime, 0.01f, 0.1f);
                }
            }

            ImGui::SeparatorText("Clip A");
            ImGui::InputInt("Clip A", &anim.m_clipIndexA, 1, 10);
            ImGui::InputFloat("Clip A start", &anim.m_startTimeA, 0.01f, 0.1f);
            ImGui::InputFloat("Clip A speed", &anim.m_speedA, 0.01f, 0.1f);

            ImGui::SeparatorText("Clip B");
            ImGui::Checkbox("Blend animation", &anim.m_blend);

            if (anim.m_blend) {
                ImGui::InputFloat("Blend factor", &anim.m_blendFactor, 0.01f, 0.1f);

                ImGui::InputInt("Clip B", &anim.m_clipIndexB, 1, 10);
                ImGui::InputFloat("Clip B start", &anim.m_startTimeB, 0.01f, 0.1f);
                ImGui::InputFloat("Clip B speed", &anim.m_speedB, 0.01f, 0.1f);
            }

            if (assets.glslUseDebug) {
                ImGui::SeparatorText("Joint visualization");

                ImGui::Checkbox("Joint debug", &anim.m_debugJointWeight);
                ImGui::InputInt("Joint index", &anim.m_jointIndex, 1, 10);
            }
        }
    }

    void NodeTool::processInputs(
        const InputContext& ctx)
    {
        const auto& input = ctx.getInput();
        const auto& inputState = ctx.getInputState();

        if (inputState.mouseLeft != m_state.m_wasMouseLeft &&
            inputState.mouseLeft == GLFW_PRESS &&
            ctx.getInput().allowMouse())
        {
            if (inputState.ctrl)
            {
                handleSelectNode(ctx);
            }
            //else if (inputState.shift)
            //{
            //    shoot(ctx, scene, input, inputState);
            //}
        }

        m_state.m_wasMouseLeft = inputState.mouseLeft;
    }

    void NodeTool::handleSelectNode(
        const InputContext& ctx)
    {
        auto* scene = ctx.getScene();
        if (!scene) return;

        const auto& input = ctx.getInput();
        const auto& inputState = ctx.getInputState();
        auto& window = m_editor.getWindow();

        auto& nodeRegistry = *ctx.getRegistry()->m_nodeRegistry;
        auto& selectionRegistry = *ctx.getRegistry()->m_selectionRegistry;
        auto& commandEngine = script::CommandEngine::get();

        auto& dbg = debug::DebugContext::modify();

        const bool selectMode = inputState.ctrl;

        ki::node_id nodeId = scene->getObjectID(
            ctx.toRenderContext(),
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
        //            ctx.getRegistry()->m_dispatcherWorker->send(evt);
        //        }

        //        node = nullptr;
        //    }
        //}
        //else if (cameraMode) {
        //    // NOTE KI null == default camera
        //    event::Event evt { event::Type::camera_activate };
        //    evt.body.node.target = node->getId();
        //    ctx.getRegistry()->m_dispatcherView->send(evt);

        //    node = nullptr;
        //}
    }

    void NodeTool::onSelectNode(
        const gui::FrameContext& ctx,
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
        const gui::FrameContext& ctx,
        pool::NodeHandle nodeHandle)
    {
        auto* node = m_state.m_selectedNode.toNode();
        if (!node) return;

        event::Event evt{ event::Type::node_remove };
        auto& body = evt.body.node = {
            .target = m_state.m_selectedNode.toId(),
        };
        ctx.getRegistry()->m_dispatcherWorker->send(evt);
    }

    void NodeTool::onCloneNode(
        const gui::FrameContext& ctx,
        pool::NodeHandle nodeHandle)
    {
        auto* node = m_state.m_selectedNode.toNode();
        if (!node) return;

        const auto& state = node->getState();
        const auto* type = node->getType();

        {
            auto* parent = node->getParent();
            ki::node_id parentId{ node->getParentHandle().toId() };
            glm::vec3 pos{ state.getPosition() };
            glm::quat quat{ state.getRotation() };
            glm::vec3 scale{ state.getScale() };

            model::CreateState state{
                pos,
                scale,
                quat };

            model::CompositeBuilder builder{ NodeRegistry::get() };
            if (builder.build(parentId, 0, type, state)) {
                auto rootHandle = builder.asyncAttach(ctx.getRegistry());

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

    void NodeTool::updateSocket(
        model::Node* node,
        mesh::Mesh* mesh,
        animation::RigContainer* rig,
        animation::RigSocket* socket)
    {
        socket->updateTransforms();

        // NOTE KI MUST update palette if animation is not playing
        {
            auto& animationSystem = animation::AnimationSystem::get();
            auto& socketRegistry = *animationSystem.m_socketRegistry.get();
            std::lock_guard lockSockets(socketRegistry.m_lock);

            const auto* rigNode = rig->getNode(socket->m_nodeIndex);

            const auto& state = node->getState();
            const auto socketIndex = state.m_socketBaseIndex + socket->m_index;

            auto socketPalette = socketRegistry.modifyRange(socketIndex , 1);
            socketPalette[0] = socket->calculateGlobalTransform(rigNode->m_globalTransform);
            socketRegistry.markDirty(socketIndex, 1);
        }
    }
}
