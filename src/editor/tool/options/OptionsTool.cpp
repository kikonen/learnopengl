#include "OptionsTool.h"

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
#include "registry/ControllerRegistry.h"

#include "scene/Scene.h"

#include "editor/EditorFrame.h"

class PawnController;

namespace editor
{
    OptionsTool::OptionsTool(EditorFrame& editor)
        : Tool{ editor, "Options" }
    {
    }

    OptionsTool::~OptionsTool() = default;

    void OptionsTool::drawMenuImpl(
        const render::RenderContext& ctx,
        Scene* scene,
        debug::DebugContext& dbg)
    {
        if (ImGui::BeginMenu("Options"))
        {
            ImGui::Checkbox("Options|Draw debug", &dbg.m_drawDebug);
            ImGui::EndMenu();
        }
    }

    void OptionsTool::drawImpl(
        const render::RenderContext& ctx,
        Scene* scene,
        debug::DebugContext& dbg)
    {
        if (ImGui::CollapsingHeader("Options"))
        {
            renderOptions(ctx, dbg);
        }
    }

    void OptionsTool::processInputs(
        const render::RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState)
    {
    }


    void OptionsTool::renderOptions(
        const render::RenderContext& ctx,
        debug::DebugContext& dbg)
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
