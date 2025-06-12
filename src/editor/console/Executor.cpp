#include "Executor.h"

#include <iostream>

#include <imgui.h>
#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/util.h"
#include "util/log.h"

#include "script/lua_binding.h"
#include "script/ScriptSystem.h"

#include "pool/IdGenerator.h"

namespace {
    IdGenerator<int> ID_GENERATOR;

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
    Executor::Executor()
    {
    }

    Executor::~Executor() = default;

    int Executor::addCommand(CommandItem item)
    {
        item.m_id = ID_GENERATOR.nextId();
        std::lock_guard lock(m_lock);
        m_pending.push_back(item);
        return item.m_id;
    }

    std::vector<CommandItem> Executor::getResults()
    {
        std::vector<CommandItem> results;
        {
            std::lock_guard lock(m_lock);
            results = m_results;
            m_results.clear();
        }
        return results;
    }

    int Executor::execute()
    {
        int resultCount;

        {
            std::lock_guard lock(m_lock);
            m_commands = m_pending;
            m_pending.clear();
            resultCount = m_results.size();
        }

        if (m_commands.empty()) return resultCount;

        {
            auto& scriptSystem = script::ScriptSystem::get();
            for (auto& item : m_commands)
            {
                const auto& result = scriptSystem.execRepl(item.m_command);
                item.m_result = convert(result);
            }
        }

        {
            std::lock_guard lock(m_lock);
            for (auto& item : m_commands)
            {
                m_results.push_back(item);
            }
            resultCount = m_results.size();
        }

        m_commands.clear();

        return resultCount;
    }
}
