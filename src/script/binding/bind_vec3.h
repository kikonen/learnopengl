#pragma once

#include <glm/glm.hpp>

#include <functional>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <sol/forward.hpp>

// https://blog.rubenwardy.com/2020/07/22/sol3-vector-extension/
//
// NOtE KI this does not seem to work if not in "sol" namespace
//
namespace sol {
    //struct lua_type_of<glm::vec3>
    //    : std::integral_constant<sol::type, sol::type::table> {
    //};

    inline bool sol_lua_check(
        sol::types<glm::vec3>,
        lua_State* L,
        int index,
        std::function<sol::check_handler_type>&& handler,
        sol::stack::record& tracking)
    {
        int absoluteIndex = lua_absindex(L, index);
        if (!sol::stack::check<sol::table>(L, absoluteIndex, handler)) {
            tracking.use(1);
            return false;
        }

        sol::stack::get_field(L, 1, absoluteIndex);
        bool x = sol::stack::check<float>(L, -1);

        sol::stack::get_field(L, 2, absoluteIndex);
        bool y = sol::stack::check<float>(L, -1);

        sol::stack::get_field(L, 3, absoluteIndex);
        bool z = sol::stack::check<float>(L, -1);

        sol::stack::pop_n(L, 3);

        tracking.use(1);
        return x && y && z;
    };

    inline glm::vec3 sol_lua_get(sol::types<glm::vec3>, lua_State* L, int index,
        sol::stack::record& tracking) {
        int absoluteIndex = lua_absindex(L, index);

        sol::table table = sol::stack::get<sol::table>(L, absoluteIndex);
        float x = table[1];
        float y = table[2];
        float z = table[3];

        tracking.use(1);

        return { x, y, z };
    };

    //inline int sol_lua_push(sol::types<glm::vec3>, lua_State* L, const glm::vec3& pos) {
    //    lua_createtable(L, 0, 3);

    //    lua_getglobal(L, "Vector");
    //    lua_setmetatable(L, -2);

    //    sol::stack_table vec(L);
    //    vec["x"] = pos.x;
    //    vec["y"] = pos.y;
    //    vec["z"] = pos.z;

    //    return 1;
    //};
}
