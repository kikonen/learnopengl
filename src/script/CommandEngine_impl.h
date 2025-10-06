#pragma once

#include <iostream>

#include <fmt/format.h>

#include "util/Log.h"

#include "CommandEngine.h"

#include "CommandHandle.h"
#include "CommandEntry.h"

namespace script {
    template<typename T>
    script::command_id CommandEngine::addCommand(
        script::command_id afterId,
        T&& cmd) noexcept
    {
        try {
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
        catch (const std::runtime_error& ex) {
            KI_CRITICAL(fmt::format("COMMAND_ENGINE: ADD_COMMAND_FAIL - {}", ex.what()));
            return -1;
        }
        catch (const std::exception& ex) {
            KI_CRITICAL(fmt::format("COMMAND_ENGINE: ADD_COMMAND_FAIL - {}", ex.what()));
            return -1;
        }
        catch (const std::string& ex) {
            KI_CRITICAL(fmt::format("COMMAND_ENGINE: ADD_COMMAND_FAIL - {}", ex));
            return -1;
        }
        catch (const char* ex) {
            KI_CRITICAL(fmt::format("COMMAND_ENGINE: ADD_COMMAND_FAIL - {}", ex));
            return -1;
        }
        catch (...) {
            KI_CRITICAL(fmt::format("COMMAND_ENGINE: ADD_COMMAND_FAIL - {}", "UNKNOWN_ERROR"));
            return -1;
        }
    }
}
