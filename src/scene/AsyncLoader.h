#pragma once

#include <future>
#include <mutex>

#include "asset/Assets.h"
#include "asset/ShaderRegistry.h"

#include "Scene.h"


class AsyncLoader
{
public:
    AsyncLoader(
        ShaderRegistry& shaders,
        const Assets& assets);

    virtual void setup();

    void addLoader(std::function<void()> loader);
    const std::future<void>& getLoader(unsigned int index);

    std::shared_ptr<Shader> getShader(
        const std::string& name);

    std::shared_ptr<Shader> getShader(
        const std::string& name,
        const std::vector<std::string>& defines);

    // wait for loading of node
    // @return node null if not found
    Node* waitNode(const uuids::uuid& id);

    void waitForReady();

public:
    ShaderRegistry& shaders;
    const Assets& assets;
    std::shared_ptr<Scene> scene = nullptr;

protected:
    // https://stackoverflow.com/questions/20126551/storing-a-future-in-a-list
    std::vector<std::future<void>> loaders;

    int loadedCount = 0;

    std::condition_variable waitCondition;

    std::mutex load_lock;
};
