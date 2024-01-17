#pragma once

#include <future>
#include <mutex>

#include "asset/Assets.h"

class AsyncLoader
{
public:
    AsyncLoader(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    virtual void setup();

    void addLoader(
        std::shared_ptr<std::atomic<bool>> sceneAlive,
        std::function<void()> loader);

    void waitForReady();

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    int m_startedCount = 0;
    int m_loadedCount = 0;

    int m_asyncWaiterCount = 0;

    std::condition_variable m_waitCondition;

    std::mutex m_load_lock{};
};
