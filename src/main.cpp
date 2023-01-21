#include <iostream>
#include <fstream>
#include <exception>

#include <fmt/format.h>

#include <entt/entt.hpp>

#include "Engine.h"
#include "Test6.h"

int runEngine() {
    auto engine = std::make_unique<Test6>();

    try {
        KI_INFO("START: ENGINE INIT");
        if (engine->init()) {
            KI_INFO("FAIL: ENGINE INIT");
            return -1;
        }
        KI_INFO("DONE: ENGINE INIT");

        engine->run();
    }
    catch (const std::exception& ex) {
        KI_CRITICAL(ex.what());
        int x = 0;
    }
    catch (...) {
        KI_CRITICAL("UNKNOWN_ERROR");
        int x = 0;
        throw;
    }

    // HACK KI wait for a bit for pending worker threads to die
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return 0;
}

int main()
{
    entt::registry registry;
    Log::init();
    KI_INFO_OUT("START");

    runEngine();

    KI_INFO_OUT("DONE");

    if (false) {
        std::cout << "PRESS [ENTER] TO CLOSE";
        std::cin.get();
    }

    return 0;
}
