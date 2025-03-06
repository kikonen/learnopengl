#pragma once

#include "ConsoleState.h"

#include "gui/Frame.h"

namespace script
{
    class ScriptEngine;
}

namespace editor
{
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

    class ConsoleFrame : public Frame {
    public:
        ConsoleFrame(Window& window);
        ~ConsoleFrame();

        void prepare(const PrepareContext& ctx) override;

        void draw(const RenderContext& ctx) override;

        int textEditCallback(ImGuiInputTextCallbackData* data);

    private:
        void renderMenuBar();
        void renderHistory();
        void renderInput();

        ScriptResult exec(const std::string& script);

    private:
        script::ScriptEngine* m_scriptEngine{ nullptr };

        ConsoleState m_state;
    };
}
