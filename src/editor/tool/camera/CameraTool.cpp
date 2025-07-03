#include "CameraTool.h"

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

#include "render/FrameBuffer.h"

#include "render/RenderContext.h"

#include "controller/PawnController.h"
#include "controller/CameraZoomController.h"

#include "registry/NodeRegistry.h"
#include "registry/ControllerRegistry.h"

#include "scene/Scene.h"

#include "editor/EditorFrame.h"

class PawnController;

namespace editor
{
    CameraTool::CameraTool(EditorFrame& editor)
        : Tool{ editor }
    {
    }

    CameraTool::~CameraTool() = default;

    void CameraTool::draw(
        const RenderContext& ctx,
        Scene* scene,
        render::DebugContext& dbg)
    {
        if (ImGui::CollapsingHeader("Camera"))
        {
            renderCameraDebug(ctx, dbg);
        }

    }

    void CameraTool::processInputs(
        const RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState)
    {
    }

    void CameraTool::renderCameraDebug(
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
                            event::Event evt{ event::Type::node_activate };
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
                            event::Event evt{ event::Type::camera_activate };
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
}
