#include <iostream>
#include <fstream>

#include <entt/entt.hpp>

#include "Engine.h"
#include "Test6.h"


int main()
{
    entt::registry registry;
    Log::init();
    KI_INFO("START");

    auto engine = std::make_unique<Test6>();

    KI_INFO("START: ENGINE INIT");
    if (engine->init()) {
        KI_INFO("FAIL: ENGINE INIT");
        return -1;
    }
    KI_INFO("DONE: ENGINE INIT");

    engine->run();

    KI_INFO("DONE");

    if (false) {
        std::cout << "PRESS [ENTER] TO CLOSE";
        std::cin.get();
    }

    return 0;
}
