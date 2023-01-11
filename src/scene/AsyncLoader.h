#pragma once

#include <future>
#include <mutex>

#include "asset/Assets.h"

class AsyncLoader
{
public:
    AsyncLoader(
        const Assets& assets);

    virtual void setup();

    void addLoader(std::function<void()> loader);

    // wait for loading of node
    // @return node null if not found
    //Node* waitNode(const uuids::uuid& id, bool async);

    void waitForReady();

private:
    const Assets& m_assets;

    int m_startedCount = 0;
    int m_loadedCount = 0;

    int m_asyncWaiterCount = 0;

    std::condition_variable m_waitCondition;

    std::mutex m_load_lock;
};
