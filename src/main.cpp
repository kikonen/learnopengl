#include <iostream>
#include <fstream>
#include <exception>

#include "asset/AABB.h"

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
    {
        const AABB quad = { glm::vec3{ -1.f, -1.f, 0.f }, glm::vec3{ 1.f, 1.f, 0.f }, true };
        const auto& min = quad.m_min;
        const auto& max = quad.m_max;
        auto radius = glm::length(min - max) * 0.5f;
        auto center = (max + min) * 0.5f;

        std::cout << "QUAD: center={" << center.x << "," << center.x << "," << center.x << "}, radius=" << radius << "\n";
    }
    {
        const AABB quad = { glm::vec3{ -1.f, -1.f, -1.f }, glm::vec3{ 1.f, 1.f, 1.f }, true };
        const auto& min = quad.m_min;
        const auto& max = quad.m_max;
        auto radius = glm::length(min - max) * 0.5f;
        auto center = (max + min) * 0.5f;

        std::cout << "CUBE: center={" << center.x << "," << center.x << "," << center.x << "}, radius=" << radius << "\n";
    }

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
