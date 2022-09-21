#include "ShaderRegistry.h"

ShaderRegistry::ShaderRegistry()
{
}

ShaderRegistry::~ShaderRegistry()
{
    KI_INFO_SB("SHADER_REGISTRY: delete");
    for (auto& e : shaders) {
        KI_INFO_SB("SHADER_REGISTRY: delete SHADER " << e.second->shaderName);
    }
    shaders.clear();
}

std::shared_ptr<Shader> ShaderRegistry::getShader(
    const Assets& assets,
    const std::string& name)
{
    return getShader(assets, name, "", {});
}

std::shared_ptr<Shader> ShaderRegistry::getShader(
    const Assets& assets,
    const std::string& name,
    const std::vector<std::string>& defines)
{
    return getShader(assets, name, "", defines);
}

std::shared_ptr<Shader> ShaderRegistry::getShader(
    const Assets& assets,
    const std::string& name,
    const std::string& geometryType,
    const std::vector<std::string>& defines)
{
    std::lock_guard<std::mutex> lock(shaders_lock);

    std::string key = name + "_" + geometryType;

    for (auto& x : defines) {
        key += "_" + x;
    }

    std::shared_ptr<Shader> shader = nullptr;
    {
        const auto& e = shaders.find(key);
        if (e != shaders.end()) {
            shader = e->second;
        }
    }

    if (!shader) {
        shader = std::make_shared<Shader>(assets, key, name, geometryType, defines);
        shaders[key] = shader;
        shader->load();
    }

    return shader;
}

void ShaderRegistry::validate()
{
    for (auto& e : shaders) {
        auto& shader = e.second;
        assert(shader->boundCount() == 0);
    }
}
