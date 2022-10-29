#include <iostream>
#include <fstream>
#include <exception>

#include <fmt/format.h>

#include <entt/entt.hpp>

#include "Engine.h"
#include "Test6.h"

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

    runEngine();

    KI_INFO("DONE");

    if (false) {
        std::cout << "PRESS [ENTER] TO CLOSE";
        std::cin.get();
    }

    return 0;
}
