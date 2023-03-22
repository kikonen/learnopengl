#include "ProgramRegistry.h"

#include "fmt/format.h"

#include "asset/Program.h"

ProgramRegistry::ProgramRegistry(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive)
    : m_assets(assets),
    m_alive(alive)
{
}

ProgramRegistry::~ProgramRegistry()
{
    KI_INFO("PROGRAM_REGISTRY: delete");
    for (auto& e : m_programs) {
        KI_INFO(fmt::format(
            "PROGRAM_REGISTRY: delete PROGRAM {}",
            e.second->m_key));
    }
    m_programs.clear();
}

Program* ProgramRegistry::getProgram(
    const std::string& name)
{
    return getProgram(name, false, "", {});
}

Program* ProgramRegistry::getProgram(
    const std::string& name,
    const std::map<std::string, std::string>& defines)
{
    return getProgram(name, false, "", defines);
}

Program* ProgramRegistry::getComputeProgram(
    const std::string& name,
    const std::map<std::string, std::string>& defines)
{
    return getProgram(name, true, "", defines);
}

Program* ProgramRegistry::getProgram(
    const std::string& name,
    const bool compute,
    const std::string& geometryType,
    const std::map<std::string, std::string>& defines)
{
    if (!*m_alive) return nullptr;

    std::lock_guard<std::mutex> lock(m_programs_lock);

    std::string key = name;

    if (compute) {
        key += "_CS_";
    }

    if (!geometryType.empty()) {
        key += "_" + geometryType;
    }

    for (const auto& [k, v] : defines)
        key += "_" + k + "=" + v;

    Program* program = nullptr;
    {
        const auto& e = m_programs.find(key);
        if (e != m_programs.end()) {
            program = e->second.get();
        }
    }

    if (!program) {
        m_programs[key] = std::make_unique<Program>(
            m_assets,
            key,
            name,
            compute,
            geometryType,
            defines);
        const auto& e = m_programs.find(key);
        program = e->second.get();
        program->load();
    }

    return program;
}

void ProgramRegistry::validate()
{
    for (auto& e : m_programs) {
        auto& program = e.second;
        //assert(program->boundCount() == 0);
    }
}
