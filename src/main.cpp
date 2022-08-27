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

    if (engine->init()) {
        return -1;
    }

    engine->run();

    delete engine;

    KI_INFO("DONE");

    if (false) {
        std::cout << "PRESS [ENTER] TO CLOSE";
        std::cin.get();
    }

    return 0;
}
