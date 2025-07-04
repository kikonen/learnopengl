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

#include "render/NodeDraw.h"
#include "render/FrameBuffer.h"

#include "animation/RigContainer.h"
#include "animation/RigSocket.h"

#include "mesh/LodMesh.h"
#include "mesh/ModelMesh.h"

#include "render/RenderContext.h"

#include "scene/Scene.h"

#include "registry/NodeRegistry.h"
#include "registry/ControllerRegistry.h"
#include "registry/SelectionRegistry.h"

#include "editor/EditorFrame.h"

class PawnController;

namespace {
}

namespace editor
{
    NodeTypeTool::NodeTypeTool(EditorFrame& editor)
        : Tool{ editor}
    { }

    NodeTypeTool::~NodeTypeTool() = default;

    void NodeTypeTool::processInputs(
        const RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState)
    { }

    void NodeTypeTool::draw(
        const RenderContext& ctx,
        Scene* scene,
        render::DebugContext& dbg)
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
        const RenderContext& ctx,
        render::DebugContext& dbg)
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
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        const auto& nr = NodeRegistry::get();

        std::vector<NodeType*> types;

        const auto currType = m_state.m_selectedType.toType();
        if (ImGui::BeginCombo("Type selector", currType ? currType->getName().c_str() : nullptr)) {
            for (const auto* type : types)
            {
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
        const RenderContext& ctx,
        render::DebugContext& dbg)
    {
        auto* type= m_state.m_selectedType.toType();
        if (!type) return;

        {
            glm::vec3 rot = util::quatToDegrees(type->m_baseRotation);
            // , "%.3f", ImGuiInputTextFlags_EnterReturnsTrue
            if (ImGui::InputFloat3("Type base rotation", glm::value_ptr(rot))) {
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
            if (ImGui::InputFloat3("Type base scale", glm::value_ptr(scale))) {
                type->m_baseScale = scale;
            }
        }

        //if (ImGui::Button("Delete node"))
        //{
        //    onDeleteNode(ctx, m_state.m_selectedNode);
        //}
    }

    void NodeTypeTool::onSelectType(
        const RenderContext& ctx,
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
}
