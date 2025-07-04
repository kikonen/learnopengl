#pragma once

#include <memory>
#include <atomic>

namespace nav
{
    class Generator;

    class NavigationMeshBuilder {
    public:
        NavigationMeshBuilder(std::shared_ptr<Generator> generator);
        ~NavigationMeshBuilder();

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
        std::atomic_bool m_running{ false };
        int m_request{ 0 };
        std::atomic<int> m_requestCount{ 0 };
    };
}
