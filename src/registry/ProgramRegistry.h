#pragma once

#include <mutex>

#include "asset/Program.h"

class Program;

class ProgramRegistry final
{
public:
    ProgramRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~ProgramRegistry();

    Program* getProgram(
        const std::string& name);

    Program* getProgram(
        const std::string& name,
        const std::map<std::string, std::string>& defines);

    Program* getComputeProgram(
        const std::string& name);

    Program* getProgram(
        const std::string& name,
        const bool compute,
        const std::string& geometryType,
        const std::map<std::string, std::string>& defines);

    void validate();

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    // name + geom
    std::map<std::string, std::unique_ptr<Program>> m_programs;

    std::mutex m_programs_lock;
};
