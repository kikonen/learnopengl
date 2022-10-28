#include "AsyncLoader.h"


AsyncLoader::AsyncLoader(
    ShaderRegistry& shaders,
    const Assets& assets)
  : shaders(shaders),
    assets(assets)
{
}

void AsyncLoader::setup()
{
}

void AsyncLoader::waitForReady()
{
    std::unique_lock<std::mutex> lock(load_lock);

    bool done = loadedCount == startedCount;

    while (!done) {
        waitCondition.wait(lock);
        done = loadedCount == startedCount;
    }
}

void AsyncLoader::addLoader(std::function<void()> loader)
{
    if (!assets.asyncLoaderEnabled) {
        loader();
        return;
    }

    std::lock_guard<std::mutex> lock(load_lock);
    startedCount++;

    // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
    // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
    auto th = std::thread{
        [this, loader]() {
            if (assets.asyncLoaderDelay > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(assets.asyncLoaderDelay));

            loader();
            std::unique_lock<std::mutex> lock(load_lock);
            loadedCount++;
            waitCondition.notify_all();
        }
    };
    th.detach();
}

Shader* AsyncLoader::getShader(const std::string& name)
{
    return shaders.getShader(assets, name);
}

Shader* AsyncLoader::getShader(
    const std::string& name,
    const int materialCount,
    const std::map<std::string, std::string>& defines)
{
    return shaders.getShader(assets, name, materialCount, defines);
}
