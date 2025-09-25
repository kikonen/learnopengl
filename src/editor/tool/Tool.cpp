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

    void Tool::drawMenu(
        const render::RenderContext& ctx,
        Scene* scene,
        debug::DebugContext& dbg)
    {
        ImGui::PushID(m_toolId.c_str());
        drawMenuImpl(ctx, scene, dbg);
        ImGui::PopID();
    }

    void Tool::draw(
        const render::RenderContext& ctx,
        Scene* scene,
        debug::DebugContext& dbg)
    {
        ImGui::PushID(m_toolId.c_str());
        drawImpl(ctx, scene, dbg);
        ImGui::PopID();
    }
}
