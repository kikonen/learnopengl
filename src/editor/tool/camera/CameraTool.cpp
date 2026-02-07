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
#include "render/NodeCollection.h"

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
        : Tool{ editor, "Camera" }
    {
    }

    CameraTool::~CameraTool() = default;

    void CameraTool::drawImpl(
        const gui::FrameContext& ctx)
    {
        if (ImGui::CollapsingHeader("Camera"))
        {
            renderCamera(ctx);
        }
    }

    void CameraTool::processInputs(
        const InputContext& ctx)
    {
    }

    void CameraTool::renderCamera(
        const gui::FrameContext& ctx)
    {
        const auto& nr = NodeRegistry::get();
        const auto& cr = ControllerRegistry::get();

        // Pawn
        {
            const auto* currNode = nr.getActiveNode();
            if (ImGui::BeginCombo("Pawn selector", currNode ? currNode->getName().c_str() : nullptr)) {
                for (const auto& [nodeHandle, controllers] : cr.getControllers()) {
                    for (const auto& controller : controllers) {
                        if (!controller->isPawn()) continue;

                        const auto* node = nodeHandle.toNode();
                        if (!node) continue;

                        const auto* name = node->getName().c_str();

                        ImGui::PushID((void*)node);
                        if (ImGui::Selectable(name, node == currNode)) {
                            event::Event evt{ event::Type::node_activate };
                            evt.body.node.target = node->getId();
                            ctx.getRegistry()->m_dispatcherWorker->send(evt);
                        }
                        ImGui::PopID();
                    }
                }

                ImGui::EndCombo();
            }
        }

        // Camera
        if (auto* scene = ctx.getScene(); scene)
        {
            auto* collection = scene->getCollection();

            const auto* currNode = collection->getActiveCameraNode();
            if (ImGui::BeginCombo("Camera selector", currNode ? currNode->getName().c_str() : nullptr)) {
                for (const auto& [nodeHandle, controllers] : cr.getControllers()) {
                    for (const auto& controller : controllers) {
                        if (!controller->isCamera()) continue;

                        const auto* node = nodeHandle.toNode();
                        if (!node) continue;

                        const auto* name = node->getName().c_str();

                        ImGui::PushID((void*)node);
                        if (ImGui::Selectable(name, node == currNode)) {
                            event::Event evt{ event::Type::camera_activate };
                            evt.body.camera.target = node->getId();
                            ctx.getRegistry()->m_dispatcherView->send(evt);
                        }
                        ImGui::PopID();
                    }
                }

                ImGui::EndCombo();
            }
        }
    }
}
