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

    Shader* getShader(const std::string& name);

    Shader* getShader(
        const std::string& name,
        const int materialCount,
        const std::map<std::string, std::string>& defines);

    // wait for loading of node
    // @return node null if not found
    //Node* waitNode(const uuids::uuid& id, bool async);

    void waitForReady();

public:
    const Assets& assets;
    ShaderRegistry& shaders;

    std::shared_ptr<Scene> scene;

private:
    int startedCount = 0;
    int loadedCount = 0;

    int asyncWaiterCount = 0;

    std::condition_variable waitCondition;

    std::mutex load_lock;
};
