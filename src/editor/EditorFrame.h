#pragma once

#include <memory>

#include "gui/Frame.h"

#include "EditorState.h"


namespace editor {
    class NodeEditTool;
    class ConsoleFrame;

    class EditorFrame : public Frame
    {
    public:
        EditorFrame(std::shared_ptr<Window> window);
        virtual ~EditorFrame();

        void prepare(const PrepareContext& ctx) override;

        void processInputs(
            const RenderContext& ctx,
            Scene* scene,
            const Input& input,
            const InputState& inputState,
            const InputState& lastInputState) override;

        void draw(const RenderContext& ctx) override;

        EditorState& getState()
        {
            return m_state;
        }

    private:
        void renderMenuBar(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderStatus(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderCameraDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderBufferDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderPhysicsDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderEffectDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderLayersDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

        void renderMiscDebug(
            const RenderContext& ctx,
            render::DebugContext& dbg);

    private:
        EditorState m_state;

        std::unique_ptr<ConsoleFrame> m_consoleFrame;
        std::unique_ptr<NodeEditTool> m_nodeEditTool;
    };
}
