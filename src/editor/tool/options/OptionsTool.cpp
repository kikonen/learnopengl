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

namespace
{
    struct Rate
    {
        int frameRate;
        std::string label;
    };

    const Rate FRAME_RATES[3] = {
        //{20, "20"},
        {30, "30"},
        //{45, "45"},
        {60, "60"},
        //{90, "90"},
        {120, "120"},
    };
}

namespace editor
{
    OptionsTool::OptionsTool(EditorFrame& editor)
        : Tool{ editor, "Options" }
    {
    }

    OptionsTool::~OptionsTool() = default;

    void OptionsTool::drawMenuImpl(const gui::FrameContext& ctx)
    {
        auto& dbg = ctx.getDebug().edit();

        if (ImGui::BeginMenu("Options"))
        {
            ImGui::Checkbox("Options|Draw debug", &dbg.m_drawDebug);
            ImGui::EndMenu();
        }
    }

    void OptionsTool::drawImpl(const gui::FrameContext& ctx)
    {
        if (ImGui::CollapsingHeader("Options"))
        {
            renderOptions(ctx);
        }
    }

    void OptionsTool::processInputs(
        const InputContext& ctx)
    {
    }


    void OptionsTool::renderOptions(const gui::FrameContext& ctx)
    {
        auto& dbg = ctx.getDebug().edit();

        {
            ImGui::Spacing();
            ImGui::Separator();
            //ImGui::InputInt("Swap interval", &dbg.m_glfwSwapInterval, 1, 10);

            const auto& currLabel = std::to_string(dbg.m_targetFrameRate);
            if (ImGui::BeginCombo("Frame rate", currLabel.c_str())) {
                for (const auto& rate : FRAME_RATES) {

                    ImGui::PushID((void*)rate.frameRate);
                    const bool isSelected = dbg.m_targetFrameRate == rate.frameRate;
                    if (ImGui::Selectable(rate.label.c_str(), isSelected)) {
                        dbg.m_targetFrameRate = rate.frameRate;
                    }
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }
            ImGui::Spacing();
        }

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
            ImGui::InputFloat("GBuffer scale", &dbg.m_gBufferScale, 0.125f, 0.25f);

            ImGui::Checkbox("Frustum enabled", &dbg.m_frustumEnabled);

            ImGui::Checkbox("LOD debug enabled", &dbg.m_lodDebugEnabled);
            ImGui::Checkbox("LOD distance enabled", &dbg.m_lodDistanceEnabled);

            ImGui::Checkbox("Draw debug", &dbg.m_drawDebug);

            ImGui::Checkbox("LineMode", &dbg.m_forceLineMode);
            ImGui::Checkbox("Shadow visual", &dbg.m_shadowVisual);
        }
    }
}
