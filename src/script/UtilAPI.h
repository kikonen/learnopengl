#pragma once

#include <sol/sol.hpp>

namespace script
{
    class UtilAPI {
    public:
        UtilAPI();
        ~UtilAPI();

    public:
        double lua_sid(
            std::string id) noexcept;
    };
}
