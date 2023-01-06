#include "AsyncLoader.h"


AsyncLoader::AsyncLoader(
    ShaderRegistry& shaders,
    const Assets& assets)
  : m_shaders(shaders),
    assets(assets)
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

void AsyncLoader::addLoader(std::function<void()> loader)
{
    if (!assets.asyncLoaderEnabled) {
        loader();
        return;
    }

    std::lock_guard<std::mutex> lock(m_load_lock);
    m_startedCount++;

    // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
    // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
    auto th = std::thread{
        [this, loader]() {
            try {
                if (assets.asyncLoaderDelay > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(assets.asyncLoaderDelay));

                loader();
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

Shader* AsyncLoader::getShader(const std::string& name)
{
    return m_shaders.getShader(assets, name);
}

Shader* AsyncLoader::getShader(
    const std::string& name,
    const std::map<std::string, std::string>& defines)
{
    return m_shaders.getShader(assets, name, defines);
}

Shader* AsyncLoader::getShader(
    const std::string& name,
    const std::string& geometryType,
    const std::map<std::string, std::string>& defines)
{
    return m_shaders.getShader(assets, name, geometryType, defines);
}
