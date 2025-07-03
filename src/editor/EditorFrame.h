#pragma once

#include <memory>

#include "gui/Frame.h"

#include "EditorState.h"


namespace editor {
    class StatusTool;
    class CameraTool;
    class NodeEditTool;
    class ViewportTool;
    class DebugTool;
    class OptionsTool;

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

        void draw(
            const RenderContext& ctx,
            Scene* scene,
            render::DebugContext& dbg) override;

        EditorState& getState()
        {
            return m_state;
        }

    private:
        void renderMenuBar(
            const RenderContext& ctx,
            Scene* scene,
            render::DebugContext& dbg);

    private:
        EditorState m_state;

        std::unique_ptr<StatusTool> m_statusTool;
        std::unique_ptr<CameraTool> m_cameraTool;
        std::unique_ptr<NodeEditTool> m_nodeEditTool;
        std::unique_ptr<ViewportTool> m_viewportTool;
        std::unique_ptr<DebugTool> m_debugTool;
        std::unique_ptr<OptionsTool> m_optionsTool;

        std::unique_ptr<ConsoleFrame> m_consoleFrame;
    };
}
