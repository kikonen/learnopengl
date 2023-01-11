#include "ShaderRegistry.h"

#include "fmt/format.h"

ShaderRegistry::ShaderRegistry(const Assets& assets)
    : m_assets(assets)
{
}

ShaderRegistry::~ShaderRegistry()
{
    KI_INFO("SHADER_REGISTRY: delete");
    for (auto& e : m_shaders) {
        KI_INFO(fmt::format(
            "SHADER_REGISTRY: delete SHADER {}",
            e.second->m_key));
    }
    m_shaders.clear();
}

Shader* ShaderRegistry::getShader(
    const std::string& name)
{
    return getShader(name, "", {});
}

Shader* ShaderRegistry::getShader(
    const std::string& name,
    const std::map<std::string, std::string>& defines)
{
    return getShader(name, "", defines);
}

Shader* ShaderRegistry::getShader(
    const std::string& name,
    const std::string& geometryType,
    const std::map<std::string, std::string>& defines)
{
    std::lock_guard<std::mutex> lock(m_shaders_lock);

    std::string key = name;

    if (!geometryType.empty()) {
        key += "_" + geometryType;
    }

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
            m_assets,
            key,
            name,
            geometryType,
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
        //assert(shader->boundCount() == 0);
    }
}
