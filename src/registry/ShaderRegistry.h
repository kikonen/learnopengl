#pragma once

#include <mutex>

#include "asset/Shader.h"

class ShaderRegistry final
{
public:
    ShaderRegistry(const Assets& assets);
    ~ShaderRegistry();

    Shader* getShader(
        const std::string& name);

    Shader* getShader(
        const std::string& name,
        const std::map<std::string, std::string>& defines);

    Shader* getShader(
        const std::string& name,
        const std::string& geometryType,
        const std::map<std::string, std::string>& defines);

    void validate();

private:
    const Assets& m_assets;

    // name + geom
    std::map<std::string, std::unique_ptr<Shader>> m_shaders;

    std::mutex m_shaders_lock;
};
