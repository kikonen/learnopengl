#include "Tool.h"

#include <imgui.h>

#include "editor/EditorFrame.h"

namespace editor
{
    Tool::Tool(
        EditorFrame& editor,
        const std::string& toolId)
        : m_editor{ editor},
        m_toolId{ toolId }
    { }

    Tool::~Tool() = default;

    void Tool::drawMenu(const gui::FrameContext& ctx)
    {
        ImGui::PushID(m_toolId.c_str());
        drawMenuImpl(ctx);
        ImGui::PopID();
    }

    void Tool::draw(const gui::FrameContext& ctx)
    {
        ImGui::PushID(m_toolId.c_str());
        drawImpl(ctx);
        ImGui::PopID();
    }
}
