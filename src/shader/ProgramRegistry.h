#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <mutex>

#include "ki/size.h"

class Program;

struct UpdateContext;

class ProgramRegistry final
{
public:
    static ProgramRegistry& get() noexcept;

    ProgramRegistry();
    ProgramRegistry& operator=(const ProgramRegistry&) = delete;

    ~ProgramRegistry();

    void updateRT(const UpdateContext& ctx);

    void dirtyCheck(const UpdateContext& ctx);

    ki::program_id getProgram(
        std::string_view name);

    ki::program_id getProgram(
        std::string_view name,
        const std::map<std::string, std::string, std::less<>>& defines);

    ki::program_id getComputeProgram(
        std::string_view name,
        const std::map<std::string, std::string, std::less<>>& defines);

    ki::program_id getProgram(
        std::string_view name,
        const bool compute,
        std::string_view geometryType,
        const std::map<std::string, std::string, std::less<>>& defines);

    ki::program_id getProgramId(
        std::string_view name);

    ki::program_id getProgramId(
        std::string_view name,
        const bool compute,
        std::string_view geometryType,
        const std::map<std::string, std::string, std::less<>>& defines);

    Program* getProgram(ki::program_id id) noexcept
    {
        return m_programs[id].get();
    }

    void validate();

private:
    std::vector<std::unique_ptr<Program>> m_programs;
    std::unordered_map<std::string, ki::program_id> m_programIds;

    std::string m_debugGeometryType;
    bool m_debugWireframeOnly{ false };

    std::mutex m_programs_lock{};

    float m_elapsedSecs{ 0.f };
};
