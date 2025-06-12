#pragma once

#include <vector>
#include <mutex>

#include "CommandItem.h"

namespace editor
{
    class Executor
    {
    public:
        Executor();
        ~Executor();

        // @return command id
        int addCommand(CommandItem item);

        std::vector<CommandItem> getResults();

        // @return count of pending results
        int execute();

    private:
        std::mutex m_lock;

        std::vector<CommandItem> m_pending;
        std::vector<CommandItem> m_commands;
        std::vector<CommandItem> m_results;
    };
}
