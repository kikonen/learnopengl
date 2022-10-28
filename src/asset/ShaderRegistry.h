#pragma once

#include <mutex>

#include "Shader.h"

class ShaderRegistry final
{
public:
    ShaderRegistry();
    ~ShaderRegistry();

    Shader* getShader(
        const Assets& assets,
        const std::string& name);

    Shader* getShader(
        const Assets& assets,
        const std::string& name,
        const int materialCount,
        const std::map<std::string, std::string>& defines);

    Shader* getShader(
        const Assets& assets,
        const std::string& name,
        const std::string& geometryType,
        const int materialCount,
        const std::map<std::string, std::string>& defines);

    void validate();
private:
    // name + geom
    std::map<std::string, std::unique_ptr<Shader>> m_shaders;

    std::mutex m_shaders_lock;
};
