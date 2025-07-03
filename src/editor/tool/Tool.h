#pragma once

class RenderContext;
class Scene;
class Input;
struct InputState;

namespace render
{
    struct DebugContext;
}

namespace editor
{
    class EditorFrame;

    class Tool
    {
    public:
        Tool(EditorFrame& editor);
        virtual ~Tool();

        virtual void draw(
            const RenderContext& ctx,
            render::DebugContext& dbg) {}

        virtual void processInputs(
            const RenderContext& ctx,
            Scene* scene,
            const Input& input,
            const InputState& inputState,
            const InputState& lastInputState) { }

    protected:
        EditorFrame& m_editor;
    };
}
