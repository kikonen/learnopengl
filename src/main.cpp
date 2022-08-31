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

    Engine* engine = new Test6();
    Engine::current = engine;

    KI_INFO("START: ENGINE INIT");
    if (engine->init()) {
        KI_INFO("FAIL: ENGINE INIT");
        return -1;
    }
    KI_INFO("DONE: ENGINE INIT");

    engine->run();

    delete engine;

    KI_INFO("DONE");

    if (false) {
        std::cout << "PRESS [ENTER] TO CLOSE";
        std::cin.get();
    }

    return 0;
}
