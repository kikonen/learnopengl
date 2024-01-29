#pragma once

#include <iostream>

#include "CommandEngine.h"

#include "CommandHandle.h"
#include "CommandEntry.h"

namespace script {
    template<typename T>
    script::command_id CommandEngine::addCommand(
        script::command_id afterId,
        T&& cmd) noexcept
    {
        CommandHandle handle = CommandHandle::allocate(CommandHandle::nextId());

        {
            CommandEntry* entry = handle.toCommand();
            if (!entry) {
                std::cerr << "FATAL: pool full\n";
                return 0;
            }
            entry->afterId = afterId;
            entry->set<T>(std::move(cmd));
        }

        addPending(handle);

        return handle.toId();
    }
}
