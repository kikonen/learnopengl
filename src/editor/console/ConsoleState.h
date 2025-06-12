#pragma once

#include <string>
#include <vector>

#include "HistoryItem.h"

namespace editor
{
    struct ConsoleState
    {
        char m_inputBuffer[8192];

        std::vector<HistoryItem> m_items;
        // -1: new line, 0..History.Size-1 browsing history.
        int m_historyPos{ -1 };
        bool m_autoScroll{ true };
        bool m_scrollToBottom{ true };

        bool m_triggerCopyToClipboard{ false };
        bool m_triggerPateFromClipboard{ false };

        void clear()
        {
            clearInput();
            clearItems();
            clearTriggers();
        }

        void clearTriggers()
        {
            m_triggerCopyToClipboard = false;
            m_triggerPateFromClipboard = false;
        }


        char* getRawInput()
        {
            return m_inputBuffer;
        }

        void trimInput();

        std::string getInput()
        {
            return m_inputBuffer;
        }

        void setInput(const std::string& input)
        {
            clearInput();
            strncpy_s(m_inputBuffer, input.c_str(), sizeof(m_inputBuffer));
        }

        void clearInput()
        {
            m_inputBuffer[0] = 0;
        }

        void clearItems()
        {
            m_historyPos = -1;
            m_items.clear();
        }

        void addItem(const HistoryItem& item) {
            m_historyPos = -1;
            m_items.push_back(item);
        }

        // @return item, null if invalid index
        const HistoryItem* getItem(int index) {
            if (index < 0 || index >= m_items.size()) return nullptr;
            return &m_items[index];
        }
    };
}
