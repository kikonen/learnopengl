#include "ShaderRegistry.h"

ShaderRegistry::ShaderRegistry()
{
}

ShaderRegistry::~ShaderRegistry()
{
    KI_INFO_SB("SHADER_REGISTRY: delete");
    for (auto& e : m_shaders) {
        KI_INFO_SB("SHADER_REGISTRY: delete SHADER " << e.second->m_key);
    }
    m_shaders.clear();
}

Shader* ShaderRegistry::getShader(
    const Assets& assets,
    const std::string& name)
{
    return getShader(assets, name, "", 0, {});
}

Shader* ShaderRegistry::getShader(
    const Assets& assets,
    const std::string& name,
    const int materialCount,
    const std::map<std::string, std::string>& defines)
{
    return getShader(assets, name, "", materialCount, defines);
}

Shader* ShaderRegistry::getShader(
    const Assets& assets,
    const std::string& name,
    const std::string& geometryType,
    const int materialCount,
    const std::map<std::string, std::string>& defines)
{
    std::lock_guard<std::mutex> lock(m_shaders_lock);

    std::string key = name;

    if (!geometryType.empty()) {
        key += "_" + geometryType;
    }

    key += "_MAT_COUNT=" + std::to_string(materialCount);

    for (const auto& [k, v] : defines)
        key += "_" + k + "=" + v; 

    Shader* shader = nullptr;
    {
        const auto& e = m_shaders.find(key);
        if (e != m_shaders.end()) {
            shader = e->second.get();
        }
    }

    if (!shader) {
        m_shaders[key] = std::make_unique<Shader>(
            assets,
            key,
            name,
            geometryType,
            materialCount,
            defines);
        const auto& e = m_shaders.find(key);
        shader = e->second.get();
        shader->load();
    }

    return shader;
}

void ShaderRegistry::validate()
{
    for (auto& e : m_shaders) {
        auto& shader = e.second;
        assert(shader->boundCount() == 0);
    }
}
