#include <iostream>
#include <fstream>
#include <exception>

#include <fmt/format.h>

#include <entt/entt.hpp>

//include <luajit/lua.hpp>

#include <sol/sol.hpp>

#include "Engine.h"
#include "Test6.h"

void testLua() {
    lua_State* state = luaL_newstate();

    luaL_openlibs(state);

    lua_getglobal(state, "print");
    lua_pushnumber(state, 4.5);

    if (lua_pcall(state, 1, 1, 0)) {
        std::cout << "ERROR: " << lua_tostring(state, -1);
    }

    lua_getglobal(state, "print");
    lua_pushstring(state, "long string\nanother line\n");
    if (lua_pcall(state, 1, 1, 0)) {
        std::cout << "ERROR: " << lua_tostring(state, -1);
    }

    lua_close(state);
}

void testSol2() {
    sol::state lua;
    int x = 0;
    lua.set_function("beep", [&x] { ++x; });
    lua.script("beep()");
    assert(x == 1);

    std::cout << fmt::format("SOL: x={}\n", x);
}

int runEngine() {
    auto engine = std::make_unique<Test6>();

    KI_INFO("START: ENGINE INIT");
    if (engine->init()) {
        KI_INFO("FAIL: ENGINE INIT");
        return -1;
    }
    KI_INFO("DONE: ENGINE INIT");

    engine->run();
    return 0;
}

int main()
{
    entt::registry registry;
    Log::init();
    KI_INFO("START");

    testLua();
    testSol2();
    runEngine();

    KI_INFO("DONE");

    if (false) {
        std::cout << "PRESS [ENTER] TO CLOSE";
        std::cin.get();
    }

    return 0;
}
