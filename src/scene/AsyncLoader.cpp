#include "AsyncLoader.h"


AsyncLoader::AsyncLoader(const Assets& assets)
    : assets(assets)
{
}

void AsyncLoader::setup()
{
}

void AsyncLoader::waitForReady()
{
    std::unique_lock<std::mutex> lock(load_lock);

    bool done = false;
    while (!done) {
        waitCondition.wait(lock);
        done = loadedCount == loaders.size();
    }
}

void AsyncLoader::addLoader(std::function<void()> loader)
{
    std::lock_guard<std::mutex> lock(load_lock);
    loaders.emplace_back(std::async(std::launch::async, [this, loader]() {
        loader();
        std::unique_lock<std::mutex> lock(load_lock);
        loadedCount++;
        waitCondition.notify_all();
    }));
}

const std::future<void>& AsyncLoader::getLoader(unsigned int index)
{
    return loaders[index];
}

std::shared_ptr<Shader> AsyncLoader::getShader(const std::string& name)
{
    return scene->shaders.getShader(assets, name);
}

std::shared_ptr<Shader> AsyncLoader::getShader(const std::string& name, const std::vector<std::string>& defines)
{
    return scene->shaders.getShader(assets, name, "", defines);
}
