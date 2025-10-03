#pragma once

#include <string>

#include "gui/FrameContext.h"

#include "event/Listen.h"

struct PrepareContext;
struct InputContext;
class Scene;

namespace debug
{
    struct DebugContext;
}

namespace editor
{
    class EditorFrame;

    class Tool
    {
    public:
        Tool(EditorFrame& editor, const std::string& toolId);
        virtual ~Tool();

        virtual void prepare(const PrepareContext& ctx) {}

        void drawMenu(const gui::FrameContext& ctx);
        void draw(const gui::FrameContext& ctx);

        virtual void processInputs(
            const InputContext& ctx)
        {
        }

    protected:
        virtual void drawMenuImpl(const gui::FrameContext& ctx) { }
        virtual void drawImpl(const gui::FrameContext& ctx) {}

    protected:
        EditorFrame& m_editor;
        const std::string m_toolId;
    };
}
