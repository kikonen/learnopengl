#include "AsyncLoader.h"

#include "util/Util.h"
#include "util/Log.h"

#include "asset/Assets.h"

#include "kigl/kigl.h"

#include "AsyncCounter.h"

namespace {
    thread_local std::exception_ptr lastException = nullptr;
}

AsyncLoader::AsyncLoader()
{ }

AsyncLoader::~AsyncLoader()
{ }

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
    std::shared_ptr<std::atomic<bool>> alive,
    std::shared_ptr<std::atomic<int>> runningCount,
    std::function<void()> loader)
{
    const auto& assets = Assets::get();

    if (!*alive) return;

    if (!assets.asyncLoaderEnabled) {
        loader();
        return;
    }

    std::lock_guard lock(m_load_lock);

    (*runningCount)++;

    // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
    // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
    auto th = std::thread{
        [this, loader, alive, runningCount]()
        {
            m_startedCount++;
            AsyncAdd decRunning{ *runningCount, -1 };
            AsyncAdd addLoaded{ m_loadedCount, 1 };

            try {
                const auto& assets = Assets::get();

                if (assets.asyncLoaderDelay > 0)
                    util::sleep(assets.asyncLoaderDelay);

                if (*alive) {
                    loader();
                }
                std::unique_lock lock(m_load_lock);
                m_waitCondition.notify_all();
            } catch (const std::runtime_error& ex) {
                KI_CRITICAL(fmt::format("ASYNC_ERROR: {}", ex.what()));
                m_failedCount++;
            }
            catch (const std::exception& ex) {
                KI_CRITICAL(fmt::format("ASYNC_ERROR: {}", ex.what()));
                m_failedCount++;
            }
            catch (const std::string& ex) {
                KI_CRITICAL(fmt::format("ASYNC_ERROR: {}", ex));
                m_failedCount++;
            }
            catch (const char* ex) {
                KI_CRITICAL(fmt::format("ASYNC_ERROR: {}", ex));
                m_failedCount++;
            }
            catch (...) {
                KI_CRITICAL(fmt::format("ASYNC_ERROR: {}", "UNKNOWN_ERROR"));
                m_failedCount++;
            }
        }
    };
    th.detach();
}
