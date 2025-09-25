#include "StatusTool.h"

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

#include "registry/NodeRegistry.h"

#include "scene/Scene.h"

#include "editor/EditorFrame.h"

class PawnController;

namespace editor
{
    StatusTool::StatusTool(EditorFrame& editor)
        : Tool{ editor, "Status" }
    {
    }

    StatusTool::~StatusTool() = default;

    void StatusTool::drawImpl(
        const render::RenderContext& ctx,
        Scene* scene,
        debug::DebugContext& dbg)
    {
        //if (ImGui::CollapsingHeader("Status"))
        {
            renderStatus(ctx, dbg);
        }
    }

    void StatusTool::processInputs(
        const render::RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState)
    {
    }

    void StatusTool::renderStatus(
        const render::RenderContext& ctx,
        debug::DebugContext& dbg)
    {
        auto& window = m_editor.getWindow();

        const auto& fpsCounter = window.getEngine().getFpsCounter();
        //auto fpsText = fmt::format("{} fps", round(fpsCounter.getAvgFps()));
        auto fpsSummary = fpsCounter.formatSummary("");
        ImGui::Text(fpsSummary.c_str());
    }
}
