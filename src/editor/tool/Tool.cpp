#include "Tool.h"

#include "editor/EditorFrame.h"

namespace editor
{
    Tool::Tool(EditorFrame& editor)
        : m_editor{ editor}
    { }

    Tool::~Tool() = default;
}
