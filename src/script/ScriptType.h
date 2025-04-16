#pragma once

#include <type_traits>

namespace script
{
    enum class ScriptType : std::underlying_type_t<std::byte> {
        plain,
        module_file,
        class_file
    };
}
