#include "ShaderRegistry.h"

#include "fmt/format.h"

ShaderRegistry::ShaderRegistry(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_alive(alive)
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
    return getShader(name, false, "", {});
}

Shader* ShaderRegistry::getShader(
    const std::string& name,
    const std::map<std::string, std::string>& defines)
{
    return getShader(name, false, "", defines);
}

Shader* ShaderRegistry::getComputeShader(
    const std::string& name)
{
    return getShader(name, true, "", {});
}

Shader* ShaderRegistry::getShader(
    const std::string& name,
    const bool compute,
    const std::string& geometryType,
    const std::map<std::string, std::string>& defines)
{
    if (!*m_alive) return nullptr;

    std::lock_guard<std::mutex> lock(m_shaders_lock);

    std::string key = name;

    if (compute) {
        key += "_CS_";
    }

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
            compute,
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
