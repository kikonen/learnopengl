#pragma once

#include <string>
#include <vector>

namespace editor
{
    struct ConsoleState
    {
        char m_inputBuffer[2048];

        std::vector<std::string> m_items;
        std::vector<std::string> m_commands;
        std::vector<std::string> m_history;
        // -1: new line, 0..History.Size-1 browsing history.
        int m_istoryPos{ -1 };
        bool m_autoScroll{ true };
        bool m_scrollToBottom{ true };
    };
}
