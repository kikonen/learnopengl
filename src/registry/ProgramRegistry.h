#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <mutex>

#include "asset/Assets.h"


class Program;

class ProgramRegistry final
{
public:
    ProgramRegistry(
        const Assets& assets);

    ~ProgramRegistry();

    Program* getProgram(
        std::string_view name);

    Program* getProgram(
        std::string_view name,
        const std::map<std::string, std::string, std::less<>>& defines);

    Program* getComputeProgram(
        std::string_view name,
        const std::map<std::string, std::string, std::less<>>& defines);

    Program* getProgram(
        std::string_view name,
        const bool compute,
        std::string_view geometryType,
        const std::map<std::string, std::string, std::less<>>& defines);

    void validate();

private:
    const Assets& m_assets;

    // name + geom
    std::unordered_map<std::string, std::unique_ptr<Program>> m_programs;

    std::mutex m_programs_lock{};
};
