#pragma once

#include <future>
#include <atomic>
#include <mutex>

#include "util/Ref.h"

class AsyncLoader : public util::RefCounted<>
{
public:
    AsyncLoader();
    ~AsyncLoader();

    virtual void setup();

    void addLoader(
        std::shared_ptr<std::atomic_bool> alive,
        std::shared_ptr<std::atomic<int>> runningCount,
        std::function<void()> loader);

    void waitForReady();

private:
    std::atomic<int> m_startedCount{ 0 };
    std::atomic<int> m_loadedCount{ 0 };
    std::atomic<int> m_failedCount{ 0 };

    std::atomic<int> m_asyncWaiterCount{ 0 };

    std::condition_variable m_waitCondition;

    std::mutex m_load_lock{};
};
