#pragma once

#include <memory>

#include "gui/Frame.h"

#include "EditorState.h"

namespace editor {
    class StatusTool;
    class CameraTool;
    class NodeTypeTool;
    class NodeTool;
    class ViewportTool;
    class DebugTool;
    class OptionsTool;

    class ConsoleFrame;

    class EditorFrame : public gui::Frame
    {
    public:
        EditorFrame(const std::shared_ptr<Window>& window);
        virtual ~EditorFrame();

        void prepare(const PrepareContext& ctx) override;

        void clear() override;

        void processInputs(
            const InputContext& ctx) override;

        void draw(
            const gui::FrameContext& ctx) override;

        EditorState& getState()
        {
            return m_state;
        }

    private:
        void renderMenuBar(
            const gui::FrameContext& ctx);

        void renderToolBar(
            const gui::FrameContext& ctx);

        void onReloadScene(
            const gui::FrameContext& ctx);

        void onLoadScene(
            const gui::FrameContext& ctx);

        void onUnloadScene(
            const gui::FrameContext& ctx);

    private:
        EditorState m_state;

        std::unique_ptr<StatusTool> m_statusTool;
        std::unique_ptr<CameraTool> m_cameraTool;
        std::unique_ptr<NodeTypeTool> m_nodeTypeTool;
        std::unique_ptr<NodeTool> m_nodeTool;
        std::unique_ptr<ViewportTool> m_viewportTool;
        std::unique_ptr<DebugTool> m_debugTool;
        std::unique_ptr<OptionsTool> m_optionsTool;

        std::unique_ptr<ConsoleFrame> m_consoleFrame;
    };
}
