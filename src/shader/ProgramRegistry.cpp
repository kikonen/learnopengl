#include "ProgramRegistry.h"

#include "fmt/format.h"

#include "Program.h"

namespace {
    static ProgramRegistry g_registry;
}

ProgramRegistry& ProgramRegistry::get() noexcept
{
    return g_registry;
}

ProgramRegistry::ProgramRegistry()
{
    // NOTE KI no vector memory alloc, no lock needed during runtime
    m_programs.reserve(255);
    m_programs.emplace_back();
}

ProgramRegistry::~ProgramRegistry()
{
    KI_INFO("PROGRAM_REGISTRY: delete");
    for (auto& e : m_programs) {
        KI_INFO(fmt::format(
            "PROGRAM_REGISTRY: delete PROGRAM {}",
            e->m_key));
    }
    m_programs.clear();
}

ki::program_id ProgramRegistry::getProgram(
    std::string_view name)
{
    return getProgram(name, false, "", {});
}

ki::program_id ProgramRegistry::getProgram(
    std::string_view name,
    const std::map<std::string, std::string, std::less<>>& defines)
{
    return getProgram(name, false, "", defines);
}

ki::program_id ProgramRegistry::getComputeProgram(
    std::string_view name,
    const std::map<std::string, std::string, std::less<>>& defines)
{
    return getProgram(name, true, "", defines);
}

ki::program_id ProgramRegistry::getProgram(
    std::string_view name,
    const bool compute,
    std::string_view geometryType,
    const std::map<std::string, std::string, std::less<>>& defines)
{
    std::lock_guard lock(m_programs_lock);

    std::string key{ name };

    if (compute) {
        key += "_CS_";
    }

    if (!geometryType.empty()) {
        key += "_";
        key +=geometryType;
    }

    for (const auto& [k, v] : defines)
        key += "_" + k + "=" + v;

    Program* program = nullptr;
    {
        const auto& e = m_programIds.find(key);
        if (e != m_programIds.end()) {
            program = m_programs[e->second].get();
        }
    }

    if (!program) {
        ki::program_id programId = static_cast<ki::program_id>(m_programs.size());
        m_programs.push_back(std::make_unique<Program>(
            programId,
            key,
            name,
            compute,
            geometryType,
            defines));
        m_programIds[key] = programId;
        const auto& e = m_programIds.find(key);
        program = m_programs[e->second].get();
        program->load();
    }

    return program->m_id;
}

ki::program_id ProgramRegistry::getProgramId(
    std::string_view name)
{
    return getProgram(name);
}

ki::program_id ProgramRegistry::getProgramId(
    std::string_view name,
    const bool compute,
    std::string_view geometryType,
    const std::map<std::string, std::string, std::less<>>& defines)
{
    return getProgram(name, compute, geometryType, defines);
}

void ProgramRegistry::validate()
{
    for (auto& program : m_programs) {
        //assert(program->boundCount() == 0);
    }
}
