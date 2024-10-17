#include "ShaderSource.h"

#include <fstream>
#include <sstream>
#include <filesystem>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/Util.h"

#include "asset/Assets.h"

#include "Program.h"
#include "FileEntryCache.h"

namespace {
    const std::string INC_GLOBALS{ "globals.glsl" };
    const std::string INC_GLOBAL_UTILS{ "global_utils.glsl" };
}

ShaderSource::ShaderSource()
    : m_required{ false }
{
}

ShaderSource::ShaderSource(
    bool required,
    std::string path)
    : m_required{ required },
    m_path{ path }
{}

bool ShaderSource::modified() const
{
    bool modified = false;
    auto& fileCache = FileEntryCache::get();
    for (const auto& fe : m_files) {
        modified |= fileCache.isModified(fe->m_id);
    }
    return modified;
}

bool ShaderSource::exists() const
{
    bool exists = true;
    for (const auto& fe : m_files) {
        exists &= fe->exists();
    }
    return exists;
}

void ShaderSource::clear() {
    m_source.clear();
}

void ShaderSource::load(const Program& program)
{
    m_files.clear();
    m_source = loadSource(m_path, !m_required, program);
}

std::string ShaderSource::loadSource(
    const std::string& path,
    bool optional,
    const Program& program)
{
    std::vector<std::string> lines = loadSourceLines(path, optional, program);

    std::stringstream sb;

    for (auto& line : lines) {
        sb << line << std::endl;
    }

    std::string src = sb.str();
    KI_TRACE(fmt::format("== {} ===\n{}", path, src));

    return src;
}

std::vector<std::string> ShaderSource::loadSourceLines(
    const std::string& path,
    bool optional,
    const Program& program)
{
    FileEntry* fileEntry = FileEntryCache::get().getEntry(path);

    if (!fileEntry) {
        if (!optional) {
            std::string msg = fmt::format("ERROR: FILE_NOT_EXIST: {}", path);
            KI_WARN_OUT(msg);
            return {
                fmt::format("// {}", msg)
            };
        }

        return {};

        //if (optional) {
        //    return {};
        //}
        //throw std::runtime_error{ msg };
    }

    m_files.push_back(fileEntry);

    std::vector<std::string> lines;
    try {
        std::ifstream file;
        file.exceptions(std::ifstream::badbit);
        file.open(path);

        std::string line;
        int lineNumber = 1;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string k;
            std::string v1;
            std::string v2;
            std::string v3;
            ss >> k;
            ss >> v1 >> v2 >> v3;

            if (k == "#version") {
                lines.push_back(line);
                lines.push_back("#extension GL_ARB_bindless_texture : require");
                appendDefines(lines, program);

                // Globals
                for (auto& l : processInclude(INC_GLOBALS, lineNumber, program)) {
                    lines.push_back(l);
                }

                // Global utility functions
                for (auto& l : processInclude(INC_GLOBAL_UTILS, lineNumber, program)) {
                    lines.push_back(l);
                }

                //lines.push_back("#line " + std::to_string(lineNumber + 1) + " " + std::to_string(lineNumber + 1));
            }
            else if (k == "#include") {
                for (auto& l : processInclude(v1, lineNumber, program)) {
                    lines.push_back(l);
                }
                //lines.push_back("#line " + std::to_string(lineNumber + 1) + " " + std::to_string(lineNumber + 1));
            }
            else {
                lines.push_back(line);
            }
            lineNumber++;
        }

        KI_INFO(fmt::format("FILE_LOADED: {}", path));

        file.close();
    }
    catch (std::ifstream::failure e) {
        if (!optional) {
            KI_ERROR(fmt::format(
                "PROGRAM_ERROR: FILE_NOT_SUCCESFULLY_READ program={}, path={}",
                program.m_programName, path));
        }
        else {
            KI_DEBUG(fmt::format(
                "PROGRAM_ERROR: FILE_NOT_SUCCESFULLY_READ program={}, path={}",
                program.m_programName, path));
        }
        throw e;
    }

    return lines;
}

std::vector<std::string> ShaderSource::processInclude(
    std::string_view includePath,
    int lineNumber,
    const Program& program)
{
    const auto& assets = Assets::get();

    std::string simplifiedPath{ includePath };
    if (simplifiedPath.starts_with('"'))
        simplifiedPath = simplifiedPath.substr(1, simplifiedPath.length() - 1);
    if (simplifiedPath.ends_with('"'))
        simplifiedPath = simplifiedPath.substr(0, simplifiedPath.length() - 1);

    const auto& path = util::joinPathExt(
        assets.shadersDir,
        "",
        "_",
        simplifiedPath);

    std::vector<std::string> lines = loadSourceLines(path, false, program);

    std::vector<std::string> result;
    //result.push_back("#line 1 " + std::to_string(lineNumber));
    result.push_back("// [START " + simplifiedPath + "]");
    for (auto& line : lines) {
        result.push_back(line);
    }
    result.push_back("// [END " + simplifiedPath + "]");

    return result;
}

void ShaderSource::appendDefines(
    std::vector<std::string>& lines,
    const Program& program)
{
    for (const auto& [key, value] : program.getDefines()) {
        lines.push_back(fmt::format("#define {} {}", key, value));
    }
}
