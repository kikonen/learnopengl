#pragma once

#include "ki/size.h"

#include "script/lua_binding.h"

namespace script
{
    class UtilAPI {
    public:
        UtilAPI() = delete;
        ~UtilAPI() = delete;

    public:
        static double lua_sid(
            std::string id) noexcept;

        static const std::string& lua_sid_name(
            ki::sid_t id);
    };
}
