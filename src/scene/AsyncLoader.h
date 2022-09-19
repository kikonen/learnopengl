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
        std::shared_ptr<ShaderRegistry> shaders,
        const std::shared_ptr<Assets> assets);

    virtual void setup();

    void addLoader(std::function<void()> loader);

    std::shared_ptr<Shader> getShader(
        const std::string& name);

    std::shared_ptr<Shader> getShader(
        const std::string& name,
        const std::vector<std::string>& defines);

    // wait for loading of node
    // @return node null if not found
    Node* waitNode(const uuids::uuid& id, bool async);

    void waitForReady();

public:
    std::shared_ptr<ShaderRegistry> shaders;
    const std::shared_ptr<Assets> assets;
    std::shared_ptr<Scene> scene;

private:
    int startedCount = 0;
    int loadedCount = 0;

    int asyncWaiterCount = 0;

    std::condition_variable waitCondition;

    std::mutex load_lock;
};
