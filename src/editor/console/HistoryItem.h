#pragma once

#include <string>

namespace editor {
    struct HistoryItem {
        int m_id;
        std::string m_command;
        std::string m_result;
    };
}
