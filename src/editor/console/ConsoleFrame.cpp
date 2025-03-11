#include "ConsoleFrame.h"

#include <iostream>

#include <imgui.h>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/util.h"
#include "util/log.h"

#include "script/lua_binding.h"
#include "script/ScriptEngine.h"

#include "render/RenderContext.h"
#include "render/DebugContext.h"

namespace {
    static int textEditCallbackStub(ImGuiInputTextCallbackData* data)
    {
        auto* console = static_cast<editor::ConsoleFrame*>(data->UserData);
        return console->textEditCallback(data);
    }

    // https://developercommunity.visualstudio.com/t/exception-block-is-optmized-away-causing-a-crash/253077
    static std::string convert(
        const sol::protected_function_result& result)
    {
        std::string error;
        try {
            if (!result.valid()) {
                sol::error err = result;
                return err.what();
            }

            switch (result.get_type()) {
            case sol::type::none:
                return "";
            case sol::type::lua_nil:
                return "<nil>";
            case sol::type::string:
                return result.get<std::string>();
            case sol::type::number:
                return std::to_string(result.get<double>());
            case sol::type::thread:
                return "thread";
            case sol::type::boolean:
                return result.get<bool>() ? "true" : "false";
            case sol::type::function:
                return "function";
            case sol::type::userdata:
                return "userdata";
            case sol::type::lightuserdata:
                return "lightuserdata";
            case sol::type::table:
                return "table";
            case sol::type::poly:
                return "poly";
            }

            return fmt::format("type: {}", util::as_integer(result.get_type()));
        }
        catch (const std::exception& ex) {
            error = ex.what();
        }
        catch (...) {
            error = "UNKNOWN_ERROR";
        }
        return error;
    }
}

namespace editor
{
    ConsoleFrame::ConsoleFrame(Window& window)
        : Frame{ window }
    {
        m_state.clear();
    }

    ConsoleFrame::~ConsoleFrame() = default;

    void ConsoleFrame::prepare(const PrepareContext& ctx)
    {
        m_scriptEngine = &script::ScriptEngine::get();
    }

    void ConsoleFrame::draw(const RenderContext& ctx)
    {
        const auto& assets = ctx.m_assets;
        auto& dbg = render::DebugContext::modify();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= 0 |
            ImGuiConfigFlags_NavEnableKeyboard |
            0;

        // NOTE KI don't waste CPU if Edit window is collapsed
        bool* openPtr = nullptr;
        ImGuiWindowFlags flags = 0 |
            ImGuiWindowFlags_MenuBar |
            0;

        if (!ImGui::Begin("Console", openPtr, flags)) {
            trackImGuiState(dbg);
            ImGui::End();
            return;
        }

        m_state.clearTriggers();

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        //if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        //ImGuiID dockspace_id = ImGui::GetID("learnopengl_editor");
        //ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        //}

        renderMenuBar();
        renderHistory();
        renderInput();

        trackImGuiState(dbg);

        ImGui::End();
    }

    void ConsoleFrame::renderMenuBar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Edit"))
            {
                m_state.m_triggerCopyToClipboard = ImGui::MenuItem("Copy");
                m_state.m_triggerPateFromClipboard = ImGui::MenuItem("Paste");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Options"))
            {
                ImGui::Checkbox("Auto scroll", &m_state.m_autoScroll);
                ImGui::Checkbox("Scroll to Bottom", &m_state.m_scrollToBottom);
                ImGui::EndMenu();
            }


            ImGui::EndMenuBar();
        }
    }

    void ConsoleFrame::renderHistory()
    {
        // Reserve enough left-over height for 1 separator + 1 input text
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve),
            0,
            ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::Selectable("Clear")) {
                    m_state.clearItems();
                }
                ImGui::EndPopup();
            }

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

            if (m_state.m_triggerCopyToClipboard) {
                ImGui::LogToClipboard();
            }

            for (const HistoryItem& item : m_state.m_items)
            {
                {
                    ImVec4 color = ImVec4(0.4f, 0.8f, 0.4f, 1.0f);
                    const char* cmd = item.m_command.c_str();

                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::TextUnformatted(cmd);
                    ImGui::PopStyleColor();
                }

                if (!item.m_result.empty())
                {
                    const char* line = item.m_result.c_str();

                    //if (!Filter.PassFilter(item))
                    //    continue;

                    // Normally you would store more information in your item than just a string.
                    // (e.g. make Items[] an array of structure, store color/type etc.)
                    ImVec4 color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
                    bool has_color = false;

                    if (strstr(line, "error")) {
                        color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                        has_color = true;
                    }
                    else if (strncmp(line, "# ", 2) == 0) {
                        color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
                        has_color = true;
                    }
                    else if (strstr(line, "<nil>")) {
                        color = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
                        has_color = true;
                    }

                    if (has_color) {
                        ImGui::PushStyleColor(ImGuiCol_Text, color);
                    }
                    ImGui::TextUnformatted(line);
                    if (has_color) {
                        ImGui::PopStyleColor();
                    }
                }
            }

            if (m_state.m_triggerCopyToClipboard) {
                ImGui::LogFinish();
            }

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (m_state.m_scrollToBottom || (m_state.m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())) {
                ImGui::SetScrollHereY(1.0f);
            }
            m_state.m_scrollToBottom = false;

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
    }

    void ConsoleFrame::renderInput()
    {
        // Command-line
        bool reclaim_focus = false;

        ImGuiInputTextFlags input_text_flags = 0 |
            ImGuiInputTextFlags_EnterReturnsTrue |
            ImGuiInputTextFlags_EscapeClearsAll |
            ImGuiInputTextFlags_CallbackCompletion |
            ImGuiInputTextFlags_CallbackHistory |
            0;

        if (ImGui::InputText(
            "Input",
            m_state.getRawInput(),
            sizeof(m_state.m_inputBuffer),
            input_text_flags,
            &textEditCallbackStub, (void*)this))
        {
            m_state.trimInput();
            std::string cmd = m_state.getInput();
            if (!cmd.empty()) {
                execCommand(cmd);
                m_state.clearInput();
            }
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();

        if (reclaim_focus) {
            // Auto focus previous widget
            ImGui::SetKeyboardFocusHere(-1);
        }
    }

    void ConsoleFrame::execCommand(const std::string& cmd)
    {
        HistoryItem item;
        item.m_command = cmd;

        const auto& result = m_scriptEngine->execRepl(cmd);

        item.m_result = convert(result);
        m_state.addItem(item);
    }

    int ConsoleFrame::textEditCallback(ImGuiInputTextCallbackData* data)
    {
        switch (data->EventFlag)
        {
            case ImGuiInputTextFlags_CallbackCompletion:
            {
                break;
            }
            case ImGuiInputTextFlags_CallbackHistory:
            {
                int historyPos = m_state.m_historyPos;
                const int prevHistoryPos = historyPos;

                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (historyPos == -1) {
                        historyPos = m_state.m_items.empty() ? -1 : m_state.m_items.size() - 1;
                    }
                    else if (historyPos > 0) {
                        historyPos--;
                    }
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (historyPos != -1)
                        if (++historyPos >= m_state.m_items.size()) {
                            historyPos = -1;
                        }
                }

                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prevHistoryPos != historyPos)
                {
                    m_state.m_historyPos = historyPos;

                    data->DeleteChars(0, data->BufTextLen);
                    const auto* item = m_state.getItem(historyPos);
                    if (item) {
                        data->InsertChars(0, item->m_command.c_str());
                    }
                }

                break;
            }
        }
        return 0;
    }

}
