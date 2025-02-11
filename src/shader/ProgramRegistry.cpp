#include "ProgramRegistry.h"

#include "fmt/format.h"

#include "engine/UpdateContext.h"

#include "render/DebugContext.h"

#include "Program.h"
#include "FileEntryCache.h"


namespace {
    constexpr float DIRTY_CHECK_FREQUENCY{ 4.f };
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
    for (auto& program : m_programs) {
        if (!program) continue;
        KI_INFO(fmt::format(
            "PROGRAM_REGISTRY: delete PROGRAM {}",
            program->m_key));
    }
    m_programs.clear();
}

void ProgramRegistry::updateRT(const UpdateContext& ctx)
{
    std::lock_guard lock(m_programs_lock);

    for (auto& program : m_programs) {
        if (!program) continue;
        if (program->isLoaded() && !program->isPrepared()) {
            KI_INFO_OUT(fmt::format("PROGRAM_PREPARE: {}", program->m_key));
            program->prepareRT();
        }
    }
}

void ProgramRegistry::dirtyCheck(const UpdateContext& ctx)
{
    std::lock_guard lock(m_programs_lock);

    const auto& dbg = render::DebugContext::get();

    std::vector<ki::program_id> dirty;

    if (dbg.m_geometryType != m_debugGeometryType || dbg.m_wireframeOnly != m_debugWireframeOnly) {
        m_debugGeometryType = dbg.m_geometryType;
        m_debugWireframeOnly = dbg.m_wireframeOnly;

        for (auto& program : m_programs) {
            if (!program) continue;
            if (program->setDebugGeometryType(m_debugGeometryType)) {
                dirty.push_back(program->m_id);
            }
        }
    }

    m_elapsedSecs += ctx.m_clock.elapsedSecs;

    if (!dirty.empty() || m_elapsedSecs > DIRTY_CHECK_FREQUENCY) {
        auto& fileCache = FileEntryCache::get();

        fileCache.checkModified();

        for (auto& program : m_programs) {
            if (!program) continue;
            if (!program->isLoaded()) continue;

            const auto isDirty = std::find(dirty.begin(), dirty.end(), program->m_id) != dirty.end();
            if (isDirty || program->isModified()) {
                KI_INFO_OUT(fmt::format("PROGRAM_RELOAD: {}", program->m_key));
                program->reload();
            }
        }

        //KI_INFO_OUT(fmt::format(
        //    "check_dirty: elapsed={}s, empty={}, files={}, modified={}",
        //    m_elapsedSecs, dirty.empty(), fileCache.getSize(), fileCache.getModifiedCount()));

        fileCache.clearModified();
        m_elapsedSecs = 0.f;
    }
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
    std::lock_guard lock(m_programs_lock);

    for (auto& program : m_programs) {
        if (!program) continue;
        //assert(program->boundCount() == 0);
    }
}
