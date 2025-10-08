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

    bool NavigationMeshBuilder::isRunning()
    {
        return m_running;
    }

    void NavigationMeshBuilder::start()
    {
        std::lock_guard lock{ m_lock };

        m_pendingCount++;
        if (m_running) return;
        m_alive = true;

        auto th = std::thread{
            [this]() mutable {
                try {
                    m_running = true;
                    util::markOtherThread();

                    while (m_alive && m_pendingCount > 0)
                    {
                        run();

                        if (m_alive)
                        {
                            m_pendingCount--;
                        }
                    }

                    std::lock_guard lock{ m_lock };

                    if (!m_alive)
                    {
                        m_pendingCount = 0;
                        m_handledCount = 0;
                    }

                    m_running = false;
                }
                catch (const std::exception& ex) {
                    KI_CRITICAL(fmt::format("NAV_MESH: {}", ex.what()));
                    m_pendingCount = 0;
                    m_running = false;
                }
                catch (const std::string& ex) {
                    KI_CRITICAL(fmt::format("NAV_MESH: {}", ex));
                    m_pendingCount = 0;
                    m_running = false;
                }
                catch (const char* ex) {
                    KI_CRITICAL(fmt::format("NAV_MESH: {}", ex));
                    m_pendingCount = 0;
                    m_running = false;
                }
                catch (...) {
                    KI_CRITICAL("NAV_MESH: UNKNOWN_ERROR");
                    m_pendingCount = 0;
                    m_running = false;
                }
            }
        };
        th.detach();
    }

    void NavigationMeshBuilder::stop()
    {
        std::lock_guard lock{ m_lock };
        m_alive = false;
        m_pendingCount = 0;
        m_handledCount = 0;
    }

    void NavigationMeshBuilder::run()
    {
        KI_INFO_OUT(fmt::format("NAV_MESH: start: {}", m_handledCount));
        m_generator->build(m_alive);
        KI_INFO_OUT(fmt::format("NAV_MESH: done: {}", m_handledCount));
    }
}
