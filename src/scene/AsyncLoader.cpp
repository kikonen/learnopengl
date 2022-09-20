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

Node* AsyncLoader::waitNode(const uuids::uuid& id, bool async)
{
    std::unique_lock<std::mutex> lock(load_lock);

    if (async) asyncWaiterCount++;

    // TODO KI this will stuck *FOREVER* if waiter is async
    auto node = scene->registry.getNode(id);
    bool done = loadedCount == startedCount - asyncWaiterCount;

    while (!done && !node) {
        waitCondition.wait(lock);
        done = loadedCount == startedCount - asyncWaiterCount;
        node = scene->registry.getNode(id);
    }

    if (async) asyncWaiterCount--;

    return node;
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
    std::lock_guard<std::mutex> lock(load_lock);
    startedCount++;

    // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
    // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
    auto th = std::thread{
        [this, loader]() {
            std::this_thread::sleep_for(std::chrono::milliseconds((int)(2 * 1000.f)));

            loader();
            std::unique_lock<std::mutex> lock(load_lock);
            loadedCount++;
            waitCondition.notify_all();
        }
    };
    th.detach();
}

std::shared_ptr<Shader> AsyncLoader::getShader(const std::string& name)
{
    return shaders.getShader(assets, name);
}

std::shared_ptr<Shader> AsyncLoader::getShader(
    const std::string& name,
    const std::vector<std::string>& defines)
{
    return shaders.getShader(assets, name, "", defines);
}
