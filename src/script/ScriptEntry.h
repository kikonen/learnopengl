#pragma once

#include <string>

#include <sol/sol.hpp>

#include "ScriptEntryType.h"


namespace script
{
    struct ScriptEntry {
        bool m_valid{ false };
        ScriptEntryType m_type{ ScriptEntryType::function };
        std::string m_signature;
        // module/class
        //sol::table m_table;
    };
}
