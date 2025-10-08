#pragma once

#include <memory>
#include <atomic>
#include <mutex>

namespace nav
{
    class Generator;

    class NavigationMeshBuilder {
    public:
        NavigationMeshBuilder(std::shared_ptr<Generator> generator);
        ~NavigationMeshBuilder();

        bool isRunning();

        void start();
        void stop();

        bool isRunning() const
        {
            return m_running;
        }

    private:
        void run();

    private:
        std::shared_ptr<Generator> m_generator;

        std::mutex m_lock;
        std::atomic_bool m_running{ false };
        std::atomic_bool m_alive{ true };
        int m_handledCount{ 0 };
        std::atomic<int> m_pendingCount{ 0 };
    };
}
