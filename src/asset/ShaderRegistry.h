#pragma once

#include <mutex>

#include "Shader.h"

class ShaderRegistry final
{
public:
    ShaderRegistry();
    ~ShaderRegistry();

    std::shared_ptr<Shader> getShader(
        const Assets& assets,
        const std::string& name);

    std::shared_ptr<Shader> getShader(
        const Assets& assets,
        const std::string& name,
        const std::vector<std::string>& defines);

    std::shared_ptr<Shader> getShader(
        const Assets& assets,
        const std::string& name,
        const std::string& geometryType,
        const std::vector<std::string>& defines);

    void validate();
private:
    // name + geom
    std::map<std::string, std::shared_ptr<Shader>> shaders;

    std::mutex shaders_lock;
};
