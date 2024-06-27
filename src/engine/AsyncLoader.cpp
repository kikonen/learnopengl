#include "AsyncLoader.h"

#include "util/Log.h"

#include "asset/Assets.h"

#include "kigl/kigl.h"

namespace {
    thread_local std::exception_ptr lastException = nullptr;
}

AsyncLoader::AsyncLoader(
    std::shared_ptr<std::atomic<bool>> alive)
  : m_alive(alive)
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
    const auto& assets = Assets::get();

    if (!assets.asyncLoaderEnabled) {
        loader();
        return;
    }

    std::lock_guard lock(m_load_lock);
    m_startedCount++;

    // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
    // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
    auto th = std::thread{
        [this, loader, sceneAlive]() {
            try {
                const auto& assets = Assets::get();

                if (assets.asyncLoaderDelay > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(assets.asyncLoaderDelay));

                if (*sceneAlive && *m_alive) {
                    loader();
                }
                std::unique_lock lock(m_load_lock);
                m_loadedCount++;
                m_waitCondition.notify_all();
            } catch (const std::runtime_error& ex) {
                KI_CRITICAL(fmt::format("ASYNC_ERROR: {}", ex.what()));
            }
            catch (const std::string& ex) {
                KI_CRITICAL(fmt::format("ASYNC_ERROR: {}", ex));
            }
            catch (const char* ex) {
                KI_CRITICAL(fmt::format("ASYNC_ERROR: {}", ex));
            }
            catch (...) {
                KI_CRITICAL(fmt::format("ASYNC_ERROR: {}", "UNKNOWN_ERROR"));
            }
        }
    };
    th.detach();
}
