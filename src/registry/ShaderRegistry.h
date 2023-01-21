#pragma once

#include <mutex>

#include "asset/Shader.h"

class ShaderRegistry final
{
public:
    ShaderRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~ShaderRegistry();

    Shader* getShader(
        const std::string& name);

    Shader* getShader(
        const std::string& name,
        const std::map<std::string, std::string>& defines);

    Shader* getComputeShader(
        const std::string& name);

    Shader* getShader(
        const std::string& name,
        const bool compute,
        const std::string& geometryType,
        const std::map<std::string, std::string>& defines);

    void validate();

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    // name + geom
    std::map<std::string, std::unique_ptr<Shader>> m_shaders;

    std::mutex m_shaders_lock;
};
