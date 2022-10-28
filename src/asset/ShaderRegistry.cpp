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

Shader* ShaderRegistry::getShader(
    const Assets& assets,
    const std::string& name)
{
    return getShader(assets, name, "", {});
}

Shader* ShaderRegistry::getShader(
    const Assets& assets,
    const std::string& name,
    const std::map<std::string, std::string>& defines)
{
    return getShader(assets, name, "", defines);
}

Shader* ShaderRegistry::getShader(
    const Assets& assets,
    const std::string& name,
    const std::string& geometryType,
    const std::map<std::string, std::string>& defines)
{
    std::lock_guard<std::mutex> lock(shaders_lock);

    std::string key = name + "_" + geometryType;

    if (!geometryType.empty())
        key += "_" + geometryType;

    for (const auto& [k, v] : defines)
        key += "_" + k + "=" + v; 

    Shader* shader = nullptr;
    {
        const auto& e = shaders.find(key);
        if (e != shaders.end()) {
            shader = e->second.get();
        }
    }

    if (!shader) {
        shaders[key] = std::make_unique<Shader>(assets, key, name, geometryType, defines);
        const auto& e = shaders.find(key);
        shader = e->second.get();
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
