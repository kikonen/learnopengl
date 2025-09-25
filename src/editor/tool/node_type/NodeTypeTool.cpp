#include "NodeTypeTool.h"

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

#include "model/NodeType.h"
#include "model/Node.h"
#include "model/CreateState.h"
#include "model/CompositeBuilder.h"

#include "render/NodeDraw.h"
#include "render/FrameBuffer.h"

#include "script/CommandEngine.h"
#include "script/command/SelectNode.h"

#include "animation/RigContainer.h"
#include "animation/RigSocket.h"

#include "mesh/LodMesh.h"
#include "mesh/ModelMesh.h"

#include "render/RenderContext.h"

#include "scene/Scene.h"

#include "registry/NodeTypeRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/SelectionRegistry.h"

#include "editor/EditorFrame.h"

class PawnController;

namespace {
}

namespace editor
{
    NodeTypeTool::NodeTypeTool(EditorFrame& editor)
        : Tool{ editor, "NodeType" }
    {
    }

    NodeTypeTool::~NodeTypeTool() = default;

    void NodeTypeTool::processInputs(
        const render::RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState)
    {
    }

    void NodeTypeTool::drawImpl(
        const render::RenderContext& ctx,
        Scene* scene,
        debug::DebugContext& dbg)
    {
        if (ImGui::CollapsingHeader("Type"))
        {
            renderTypeEdit(ctx, dbg);
        }

        //if (ImGui::CollapsingHeader("Animation"))
        //{
        //    renderAnimationDebug(ctx, dbg);
        //}
    }

    void NodeTypeTool::renderTypeEdit(
        const render::RenderContext& ctx,
        debug::DebugContext& dbg)
    {
        renderTypeSelector(ctx, dbg);

        {
            ImGui::Spacing();
            ImGui::Separator();

            renderTypeProperties(ctx, dbg);
            //renderRigProperties(ctx, dbg);
        }

        {
            //ImGui::Spacing();
            //ImGui::Separator();

            //renderNodeDebug(ctx, dbg);
        }
    }

    void NodeTypeTool::renderTypeSelector(
        const render::RenderContext& ctx,
        debug::DebugContext& dbg)
    {
        const auto& typeRegistry = NodeTypeRegistry::get();

        const auto& typeHandles = typeRegistry.getTypeHandles();

        const auto currType = m_state.m_selectedType.toType();
        if (ImGui::BeginCombo("Type selector", currType ? currType->getName().c_str() : nullptr)) {
            for (const auto typeHandle : typeHandles)
            {
                const auto* type = typeHandle.toType();
                if (!type) continue;

                const auto* name = type->getName().c_str();

                ImGui::PushID((void*)type);

                if (ImGui::Selectable(name, type == currType)) {
                    onSelectType(ctx, type->toHandle());
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }
    }

    void NodeTypeTool::renderTypeProperties(
        const render::RenderContext& ctx,
        debug::DebugContext& dbg)
    {
        auto* type = m_state.m_selectedType.toType();
        if (!type) return;

        {
            glm::vec3 rot = util::quatToDegrees(type->m_baseRotation);
            // , "%.3f", ImGuiInputTextFlags_EnterReturnsTrue
            if (ImGui::InputFloat3("Base rotation", glm::value_ptr(rot))) {
                type->m_baseRotation = util::degreesToQuat(rot);

                auto quat = util::degreesToQuat(rot);
                auto deg = util::quatToDegrees(quat);

                KI_INFO_OUT(fmt::format(
                    "rot={}, deg={}, quat={}",
                    rot, deg, quat));
            }
        }

        {
            glm::vec3 scale = type->m_baseScale;
            if (ImGui::InputFloat3("Base scale", glm::value_ptr(scale))) {
                type->m_baseScale = scale;
            }
        }

        if (ImGui::Button("Create"))
        {
            onCreateNode(ctx, m_state.m_selectedType);
        }
    }

    void NodeTypeTool::onSelectType(
        const render::RenderContext& ctx,
        pool::TypeHandle typeHandle)
    {
        m_state.m_selectedType = typeHandle;

        //auto& commandEngine = script::CommandEngine::get();
        //commandEngine.addCommand(
        //    0,
        //    script::SelectNode{
        //        m_state.m_selectedNode,
        //        true,
        //        false
        //    });
    }

    void NodeTypeTool::onCreateNode(
        const render::RenderContext& ctx,
        pool::TypeHandle typeHandle)
    {
        const auto* type = typeHandle.toType();

        {
            ki::node_id parentId{ 0 };
            glm::vec3 pos{ 0.f };
            glm::vec3 rot{ 0.f };
            glm::vec3 scale{ 1.f };

            model::CreateState state{
                pos,
                scale,
                util::degreesToQuat(rot) };

            model::CompositeBuilder builder{ NodeRegistry::get() };
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
