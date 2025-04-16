#pragma once

#include <type_traits>

namespace script
{
    enum class ScriptEntryType : std::underlying_type_t<std::byte> {
        none,
        function,
        //table
    };
}
