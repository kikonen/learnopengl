#pragma comment(lib, "avrt.lib")

#include <iostream>
#include <fstream>
#include <exception>
#include <thread>
#include <tuple>

#include "asset/AABB.h"

#include <fmt/format.h>

#include <entt/entt.hpp>

#include "util/Util.h"
#include "util/Log.h"
#include "util/glm_format.h"
#include "util/preciseSleep.h"
#include "util/Ref.h"

#include "engine/Engine.h"
#include "sample_app/SampleApp.h"

namespace
{
#ifdef _DEBUG
    const std::string LOG_FILE = "log/development.log";
#else
    const std::string LOG_FILE = "log/production.log";
#endif
}

int runEngine()
{
    auto engine = util::Ref<SampleApp>::create();

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
        KI_CRITICAL(fmt::format("MAIN_ERROR: {}", ex.what()));
    }
    catch (const std::string& ex) {
        KI_CRITICAL(fmt::format("MAIN_ERROR: {}", ex));
    }
    catch (const char* ex) {
        KI_CRITICAL(fmt::format("MAIN_ERROR: {}", ex));
    }
    catch (...) {
        KI_CRITICAL(fmt::format("MAIN_ERROR: {}", "UNKNOWN_ERROR"));
        int x = 0;
        throw;
    }

    // HACK KI wait for a bit for pending worker threads to die
    util::sleep(500);

    return 0;
}
//
//void testVolume() {
//    {
//        const AABB quad = { glm::vec3{ -1.f, -1.f, 0.f }, glm::vec3{ 1.f, 1.f, 0.f } };
//        const auto& min = quad.m_min;
//        const auto& max = quad.m_max;
//        auto radius = glm::length(min - max) * 0.5f;
//        auto center = (max + min) * 0.5f;
//
//        std::cout << "QUAD: center={" << center.x << "," << center.x << "," << center.x << "}, radius=" << radius << "\n";
//    }
//    {
//        const AABB quad = { glm::vec3{ -1.f, -1.f, -1.f }, glm::vec3{ 1.f, 1.f, 1.f } };
//        const auto& min = quad.m_min;
//        const auto& max = quad.m_max;
//        auto radius = glm::length(min - max) * 0.5f;
//        auto center = (max + min) * 0.5f;
//
//        std::cout << "CUBE: center={" << center.x << "," << center.x << "," << center.x << "}, radius=" << radius << "\n";
//    }
//}

int main()
{
    //testVolume();
    util::initPreciseSleep();

    // https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-stdlib
    std::ios_base::sync_with_stdio(false);

    entt::registry registry;
    Log::init(LOG_FILE);
    KI_INFO_OUT("START");

    runEngine();

    KI_INFO_OUT("DONE");

    if (false) {
        std::cout << "PRESS [ENTER] TO CLOSE";
        std::cin.get();
    }

    //util::sleep(60 * 1000);

    Log::shutdown();

    return 0;
}
