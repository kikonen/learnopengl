#include "ShaderSource.h"

#include <fstream>
#include <sstream>
#include <filesystem>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/util.h"
#include "util/file.h"

#include "asset/Assets.h"

#include "Program.h"
#include "FileEntryCache.h"

#include "render/DebugContext.h"

namespace {
    const std::string INC_GLOBALS{ "globals.glsl" };
    const std::string INC_GLOBAL_UTILS{ "global_utils.glsl" };

    std::string getTypeName(GLenum shaderType) {
        switch (shaderType) {
        case GL_COMPUTE_SHADER:
            return "COMPUTE_SHADER";
        case GL_VERTEX_SHADER:
            return "VERTEX_SHADER";
        case GL_FRAGMENT_SHADER:
            return "FRAGMENT_SHADER";
        case GL_GEOMETRY_SHADER:
            return "GEOMETRY_SHADER";
        case GL_TESS_CONTROL_SHADER:
            return "TESS_CONTROL_SHADER";
        case GL_TESS_EVALUATION_SHADER:
            return "TESS_EVALUATION_SHADER";
        }
        return "UNKNOWN";
    }
}

ShaderSource::ShaderSource()
    : ShaderSource{ false, false, "" }
{
}

ShaderSource::ShaderSource(
    bool required,
    std::string path)
    : ShaderSource{ false, required, path }
{}

ShaderSource::ShaderSource(
    bool debug,
    bool required,
    std::string path)
    : m_debug{ debug },
    m_required { required },
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

bool ShaderSource::pathExists() const
{
    return util::fileExists(m_path);
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

void ShaderSource::load(
    GLenum shaderType,
    const Program& program)
{
    m_files.clear();
    m_source = loadSource(shaderType, m_path, !m_required, program);
}

std::string ShaderSource::loadSource(
    GLenum shaderType,
    const std::string& path,
    bool optional,
    const Program& program)
{
    std::vector<std::string> lines = loadSourceLines(shaderType, path, optional, program);

    std::stringstream sb;

    for (auto& line : lines) {
        sb << line << std::endl;
    }

    std::string src = sb.str();
    KI_TRACE(fmt::format("== {} ===\n{}", path, src));

    return src;
}

std::vector<std::string> ShaderSource::loadSourceLines(
    GLenum shaderType,
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
                appendDefines(shaderType, lines, program);

                // Globals
                for (auto& l : processInclude(shaderType, INC_GLOBALS, lineNumber, program)) {
                    lines.push_back(l);
                }

                // Global utility functions
                for (auto& l : processInclude(shaderType, INC_GLOBAL_UTILS, lineNumber, program)) {
                    lines.push_back(l);
                }

                //lines.push_back("#line " + std::to_string(lineNumber + 1) + " " + std::to_string(lineNumber + 1));
            }
            else if (k == "#include") {
                for (auto& l : processInclude(shaderType, v1, lineNumber, program)) {
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
    GLenum shaderType,
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

    std::vector<std::string> lines = loadSourceLines(shaderType, path, false, program);

    std::vector<std::string> result;

    std::string key = simplifiedPath;
    key = util::toUpper(key);
    key = util::replace(key, ".", "_");
    key = util::replace(key, "/", "_");
    key = util::replace(key, "\\", "_");

    result.push_back(fmt::format("#ifndef _{} // [START {}]", key, simplifiedPath));
    result.push_back(fmt::format("#define _{}", key));

    //result.push_back("#line 1 " + std::to_string(lineNumber));
    result.push_back("// [START " + simplifiedPath + "]");
    for (auto& line : lines) {
        result.push_back(line);
    }

    result.push_back(fmt::format("#endif // [END {}]", key, simplifiedPath));

    return result;
}

void ShaderSource::appendDefines(
    GLenum shaderType,
    std::vector<std::string>& lines,
    const Program& program)
{
    const auto& dbg = render::DebugContext::get();

    lines.push_back(fmt::format("#define __{}__ 1", getTypeName(shaderType)));

    for (const auto& [key, value] : program.getDefines()) {
        lines.push_back(fmt::format("#define {} {}", key, value));
    }

    if (!dbg.m_geometryType.empty())
    {
        const auto& it = program.m_sources.find(GL_GEOMETRY_SHADER);

        KI_INFO_OUT(fmt::format("[PROGRAM: {}]: CHECK_DBG geometryType={}", program.m_key, dbg.m_geometryType));

        if (it != program.m_sources.end()) {
            KI_INFO_OUT(fmt::format(
                "[PROGRAM: {}]: CHECK_PATH path={}, exists={}, debug={}",
                program.m_key, it->second.m_path, it->second.pathExists(), it->second.m_debug));

            if (it->second.m_debug && it->second.pathExists())
            {
                KI_INFO_OUT(fmt::format(
                    "[PROGRAM: {}]: USE_DBG geometryType={}, wireframeOnly={}",
                    program.m_key, dbg.m_geometryType, dbg.m_wireframeOnly));

                const auto key = util::toUpper(dbg.m_geometryType);
                lines.push_back(fmt::format("#define USE_{} 1", key));
                if (dbg.m_wireframeOnly) {
                    lines.push_back("#define USE_ALPHA 1");
                }
            }
        }
    }
}
