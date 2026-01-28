#pragma once

#include "ConsoleState.h"

#include "gui/Frame.h"

#include "event/Listen.h"

namespace script
{
    class ScriptSystem;
}

namespace event {
    class Dispatcher;
}

namespace editor
{
    class Executor;

    struct ScriptResult
    {
        bool m_success;
        std::string m_output;

        const std::string& value() const
        {
            return m_output;
        }

        bool success() const
        {
            return m_success;
        }
    };

    class ConsoleFrame : public gui::Frame {
    public:
        ConsoleFrame(const std::shared_ptr<Window>& window);
        ~ConsoleFrame();

        void prepare(const PrepareContext& ctx) override;

        void clear() override;

        void draw(
            const gui::FrameContext& ctx) override;

        int textEditCallback(ImGuiInputTextCallbackData* data);

    private:
        void renderMenuBar(const gui::FrameContext& ctx);
        void renderHistory(const gui::FrameContext& ctx);
        void renderInput(const gui::FrameContext& ctx);

        void execCommand(const std::string& script);

    private:
        ConsoleState m_state;
        std::unique_ptr<Executor> m_executor;

        event::Dispatcher* m_dispatcherWorker{ nullptr };
        event::Dispatcher* m_dispatcherView{ nullptr };

        event::Listen m_listen_console_execute;
        event::Listen m_listen_console_complete;
    };
}
