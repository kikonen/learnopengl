#pragma once

#include <future>
#include <mutex>

#include "asset/Assets.h"
#include "registry/ShaderRegistry.h"

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

    Shader* getShader(
        const std::string& name,
        const std::string& geometryType,
        const int materialCount,
        const std::map<std::string, std::string>& defines);

    // wait for loading of node
    // @return node null if not found
    //Node* waitNode(const uuids::uuid& id, bool async);

    void waitForReady();

public:
    const Assets& assets;
    ShaderRegistry& m_shaders;

    std::shared_ptr<Scene> m_scene;

private:
    int m_startedCount = 0;
    int m_loadedCount = 0;

    int m_asyncWaiterCount = 0;

    std::condition_variable m_waitCondition;

    std::mutex m_load_lock;
};
