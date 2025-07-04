#include "NavigationMeshBuilder.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <fmt/format.h>

#include "util/Util.h"
#include "util/thread.h"
#include "util/Log.h"

#include "Generator.h"

namespace nav
{
    NavigationMeshBuilder::NavigationMeshBuilder(std::shared_ptr<Generator> generator)
        : m_generator{ generator }
    { }

    NavigationMeshBuilder::~NavigationMeshBuilder()
    {
    }

    void NavigationMeshBuilder::start()
    {
        m_requestCount++;
        if (m_running) return;

        auto th = std::thread{
            [this]() mutable {
                try {
                    m_running = true;
                    util::markWorkerThread();

                    while (m_requestCount > 0)
                    {
                        run();
                        m_request++;
                        m_requestCount--;
                    }

                    m_running = false;
                }
                catch (const std::exception& ex) {
                    KI_CRITICAL(fmt::format("NAV_MESH: {}", ex.what()));
                    m_running = false;
                }
                catch (const std::string& ex) {
                    KI_CRITICAL(fmt::format("NAV_MESH: {}", ex));
                    m_running = false;
                }
                catch (const char* ex) {
                    KI_CRITICAL(fmt::format("NAV_MESH: {}", ex));
                }
                catch (...) {
                    KI_CRITICAL("NAV_MESH: UNKNOWN_ERROR");
                    m_running = false;
                }
            }
        };
        th.detach();
    }

    void NavigationMeshBuilder::stop()
    { }

    void NavigationMeshBuilder::run()
    {
        KI_INFO_OUT(fmt::format("NAV_MESH: start: {}", m_request));
        m_generator->build();
        KI_INFO_OUT(fmt::format("NAV_MESH: done: {}", m_request));
    }
}
