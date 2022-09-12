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

Node* AsyncLoader::waitNode(const uuids::uuid& id)
{
    std::unique_lock<std::mutex> lock(load_lock);

    auto node = scene->registry.getNode(id);
    bool done = loadedCount == loaders.size();

    while (!done && !node) {
        waitCondition.wait(lock);
        done = loadedCount == loaders.size();
        node = scene->registry.getNode(id);
    }

    return node;
}

void AsyncLoader::waitForReady()
{
    std::unique_lock<std::mutex> lock(load_lock);

    bool done = loadedCount == loaders.size();

    while (!done) {
        waitCondition.wait(lock);
        done = loadedCount == loaders.size();
    }
}

void AsyncLoader::addLoader(std::function<void()> loader)
{
    std::lock_guard<std::mutex> lock(load_lock);
    loaders.emplace_back(std::async(std::launch::async, [this, loader]() {
        std::this_thread::sleep_for(std::chrono::milliseconds((int)(2 * 1000.f)));

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
    return shaders.getShader(assets, name);
}

std::shared_ptr<Shader> AsyncLoader::getShader(const std::string& name, const std::vector<std::string>& defines)
{
    return shaders.getShader(assets, name, "", defines);
}
