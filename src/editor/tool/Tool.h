#pragma once

#include <string>

namespace render
{
    class RenderContext;
}

struct PrepareContext;
class Scene;
class Input;
struct InputState;

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

        void drawMenu(
            const render::RenderContext& ctx,
            Scene* scene,
            debug::DebugContext& dbg);

        void draw(
            const render::RenderContext& ctx,
            Scene* scene,
            debug::DebugContext& dbg);

        virtual void drawMenuImpl(
            const render::RenderContext& ctx,
            Scene* scene,
            debug::DebugContext& dbg) { }

        virtual void drawImpl(
            const render::RenderContext& ctx,
            Scene* scene,
            debug::DebugContext& dbg) {}

        virtual void processInputs(
            const render::RenderContext& ctx,
            Scene* scene,
            const Input& input,
            const InputState& inputState,
            const InputState& lastInputState) { }

    protected:
        EditorFrame& m_editor;
        const std::string m_toolId;
    };
}
