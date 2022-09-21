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
        const std::vector<std::string>& defines);

    Shader* getShader(
        const Assets& assets,
        const std::string& name,
        const std::string& geometryType,
        const std::vector<std::string>& defines);

    void validate();
private:
    // name + geom
    std::map<std::string, std::unique_ptr<Shader>> shaders;

    std::mutex shaders_lock;
};
