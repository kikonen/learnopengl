#include "AsyncLoader.h"

#include "util/Log.h"
#include "ki/GL.h"

AsyncLoader::AsyncLoader(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
  : m_assets(assets),
    m_alive(alive)
{
}

void AsyncLoader::setup()
{
}

void AsyncLoader::waitForReady()
{
    std::unique_lock<std::mutex> lock(m_load_lock);

    bool done = m_loadedCount == m_startedCount;

    while (!done) {
        m_waitCondition.wait(lock);
        done = m_loadedCount == m_startedCount;
    }
}

void AsyncLoader::addLoader(
    std::shared_ptr<std::atomic<bool>> sceneAlive,
    std::function<void()> loader)
{
    if (!m_assets.asyncLoaderEnabled) {
        loader();
        return;
    }

    std::lock_guard<std::mutex> lock(m_load_lock);
    m_startedCount++;

    // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
    // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
    auto th = std::thread{
        [this, loader, sceneAlive]() {
            try {
                if (m_assets.asyncLoaderDelay > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(m_assets.asyncLoaderDelay));

                if (*sceneAlive && *m_alive) {
                    loader();
                }
                std::unique_lock<std::mutex> lock(m_load_lock);
                m_loadedCount++;
                m_waitCondition.notify_all();
            } catch (const std::runtime_error& ex) {
                KI_CRITICAL(ex.what());
                KI_BREAK();
            }
        }
    };
    th.detach();
}
