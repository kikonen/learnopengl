#include "Program.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>
#include <filesystem>

#include <fmt/format.h>

#include "asset/UBO.h"

#include "asset/MatricesUBO.h"
#include "asset/DataUBO.h"
#include "asset/BufferInfoUBO.h"
#include "asset/ClipPlaneUBO.h"
#include "asset/LightUBO.h"
#include "asset/TextureUBO.h"

#include "asset/ProgramBind.h"
#include "asset/Shader.h"
#include "asset/Uniform.h"

#include "kigl/GLState.h"



namespace {
    constexpr int LOG_SIZE = 4096;

    const std::string INC_GLOBALS{ "globals.glsl" };

    const std::string GEOM_NONE{ "" };

    int idBase = 0;

    std::mutex type_id_lock{};

    int nextID()
    {
        std::lock_guard<std::mutex> lock(type_id_lock);
        return ++idBase;
    }

    bool fileExists(std::string filepath)
    {
        std::ifstream f(filepath.c_str());
        return f.good();
    }
}

Program::Program(
    const Assets& assets,
    const std::string& key,
    const std::string& name,
    const bool compute,
    const std::string& geometryType,
    const std::map<std::string, std::string>& defines)
    : m_objectID(nextID()),
    m_assets(assets),
    m_key(key),
    m_programName(name),
    m_compute(compute),
    m_geometryType(geometryType),
    m_defines(defines)
{
    std::string basePath;
    {
        std::filesystem::path fp;
        fp /= assets.shadersDir;
        fp /= name;
        basePath = fp.string();
    }

    if (m_compute) {
        m_paths[GL_COMPUTE_SHADER] = basePath + ".cs.glsl";
        m_required[GL_COMPUTE_SHADER] = true;
    }
    else {
        m_paths[GL_VERTEX_SHADER] = basePath + ".vs";
        m_paths[GL_FRAGMENT_SHADER] = basePath + ".fs";
        if (geometryType.empty()) {
            m_paths[GL_GEOMETRY_SHADER] = basePath + ".gs.glsl";
        }
        else {
            m_paths[GL_GEOMETRY_SHADER] = basePath + "_" + geometryType + ".gs.glsl";
        }

        m_paths[GL_TESS_CONTROL_SHADER] = basePath + ".tcs.glsl";
        m_paths[GL_TESS_EVALUATION_SHADER] = basePath + ".tes.glsl";

        m_required[GL_VERTEX_SHADER] = true;
        m_required[GL_FRAGMENT_SHADER] = true;
        m_required[GL_GEOMETRY_SHADER] = !geometryType.empty();
        m_required[GL_TESS_CONTROL_SHADER] = false;
        m_required[GL_TESS_EVALUATION_SHADER] = false;

        //m_bindTexture = true;
    }
}

Program::~Program()
{
    KI_INFO(fmt::format("DELETE: program={}", m_key));
    if (m_programId != -1) {
        glDeleteProgram(m_programId);
    }
}

void Program::bind(GLState& state) const noexcept
{
    assert(m_prepared);
    state.useProgram(m_programId);
}

void Program::load()
{
    for (auto& [type, path] : m_paths) {
        m_sources[type] = loadSource(path, !m_required[type]);
    }
}

int Program::prepare(const Assets& assets)
{
    if (m_prepared) return m_prepareResult;
    m_prepared = true;

    if (createProgram()) {
        m_prepareResult = -1;
        return -1;
    }

    u_shadowIndex = std::make_unique< uniform::UInt>("u_shadowIndex", UNIFORM_SHADOW_MAP_INDEX);
    u_effect = std::make_unique< uniform::Subroutine>("u_effect", GL_FRAGMENT_SHADER, SUBROUTINE_EFFECT);

    u_nearPlane = std::make_unique< uniform::Float>("u_nearPlane", UNIFORM_NEAR_PLANE);
    u_farPlane = std::make_unique< uniform::Float>("u_farPlane", UNIFORM_FAR_PLANE);

    u_drawParametersIndex = std::make_unique< uniform::UInt>("u_drawParametersIndex", UNIFORM_DRAW_PARAMETERS_INDEX);

    u_stencilMode = std::make_unique< uniform::Int>("u_stencilMode", UNIFORM_STENCIL_MODE);

    u_effectBloomIteration = std::make_unique< uniform::UInt>("u_effectBloomIteration", UNIFORM_EFFECT_BLOOM_ITERATION);

    u_effect->init(this);

    m_prepareResult = 0;
    return m_prepareResult;
}

GLint Program::getUniformLoc(const std::string& name)
{
    const auto& e = m_uniformLocations.find(name);
    if (e != m_uniformLocations.end()) {
        return e->second;
    }

    GLint vi = glGetUniformLocation(m_programId, name.c_str());
    m_uniformLocations[name] = vi;
    if (vi < 0) {
        KI_DEBUG(fmt::format(
            "PROGRAM::MISSING_UNIFORM: {} - uniform={}",
            m_programName, name));
    }
    return vi;
}

GLint Program::getUniformSubroutineLoc(const std::string& name, GLenum shaderType)
{
    auto& map = m_subroutineLocations[shaderType];
    const auto& e = map.find(name);
    if (e != map.end()) {
        return e->second;
    }

    GLint vi = glGetSubroutineUniformLocation(m_programId, shaderType, name.c_str());
    map[name] = vi;
    if (vi < 0) {
        KI_DEBUG(fmt::format(
            "PROGRAM::MISSING_SUBROUTINE: {} - type={}, subroutine={}",
            m_programName, shaderType, name));
    }
    return vi;
}

GLint Program::getSubroutineIndex(const std::string& name, GLenum shaderType)
{
    auto& map = m_subroutineIndeces[shaderType];
    const auto& e = map.find(name);
    if (e != map.end()) {
        return e->second;
    }

    GLint vi = glGetSubroutineIndex(m_programId, shaderType, name.c_str());
    map[name] = vi;
    if (vi < 0) {
        KI_DEBUG(fmt::format(
            "PROGRAM::MISSING_SUBROUTINE: {} - type={}, subroutine={}",
            m_programName, shaderType, name));
    }
    return vi;
}

int Program::compileSource(
    GLenum shaderType,
    const std::string& shaderPath,
    const std::string& source)
{
    if (source.empty()) return -1;

    const char* src = source.c_str();

    int shaderId = glCreateShader(shaderType);

    glObjectLabel(GL_SHADER, shaderId, shaderPath.length(), shaderPath.c_str());
    glShaderSource(shaderId, 1, &src, NULL);
    glCompileShader(shaderId);

    // check for shader compile errors
    {
        int success;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[LOG_SIZE];
            glGetShaderInfoLog(shaderId, LOG_SIZE, NULL, infoLog);
            KI_ERROR(fmt::format(
                "PROGRAM:::SHADER_COMPILE_FAILED[{:#04x}] PROGRAM={}\nPATH={}\n{}",
                shaderType, m_programName, shaderPath, infoLog));
            KI_ERROR(fmt::format(
                "FAILED_SOURCE:\n-------------------\n{}\n-------------------",
                source));

            glDeleteShader(shaderId);
            shaderId = -1;

            const auto msg = fmt::format(
                "PROGRAM:::SHADER_COMPILE_FAILED[{:#04x}] PROGRAM={}, PATH={}",
                shaderType, m_programName, shaderPath);
            throw std::runtime_error{ msg };
        }
    }

    return shaderId;
}

int Program::createProgram() {
    // build and compile our shader program
    // ------------------------------------
    std::unordered_map<GLenum, GLuint> shaderIds;
    for (auto& [type, source] : m_sources) {
        shaderIds[type] = compileSource(type, m_paths[type], source);
    }

    // link shaders
    {
        m_programId = glCreateProgram();

        glObjectLabel(GL_PROGRAM, m_programId, m_programName.length(), m_programName.c_str());

        for (auto& [type, shaderId] : shaderIds) {
            if (shaderId == -1) continue;
            glAttachShader(m_programId, shaderId);
            glObjectLabel(GL_SHADER, shaderId, m_paths[type].length(), m_paths[type].c_str());
        }

        glLinkProgram(m_programId);

        // check for linking errors
        {
            int success;
            glGetProgramiv(m_programId, GL_LINK_STATUS, &success);
            if (!success) {
                char infoLog[LOG_SIZE];
                glGetProgramInfoLog(m_programId, LOG_SIZE, NULL, infoLog);
                const auto msg = fmt::format(
                    "PROGRAM::PROGRAM::LINKING_FAILED program={}\n{}",
                    m_programName, infoLog);
                KI_ERROR(msg);

                glDeleteProgram(m_programId);
                m_programId = -1;

                throw std::runtime_error{ msg };
            }
        }

        for (auto& [type, shaderId] : shaderIds) {
            if (shaderId == -1) continue;
            glDeleteShader(shaderId);
        }
    }

    if (m_programId == -1) return -1;

    initProgram();
    return 0;
}

// https://community.khronos.org/t/samplers-of-different-types-use-the-same-textur/66329/4
void Program::validateProgram() const {
    if (m_programId == -1) return;
    glValidateProgram(m_programId);

    int success;
    glGetProgramiv(m_programId, GL_VALIDATE_STATUS, &success);
    if (!success) {
        char infoLog[LOG_SIZE];
        glGetProgramInfoLog(m_programId, LOG_SIZE, NULL, infoLog);

        const auto msg = fmt::format(
            "PROGRAM::PROGRAM::VALIDATE_FAILED program={}\n{}",
            m_programName, infoLog);

        KI_ERROR(msg);

        throw std::runtime_error{ msg };
    }
}

int Program::initProgram() {
    KI_INFO_OUT(fmt::format("[PROGRAM - {}]", m_key));

    // NOTE KI set UBOs only once for program
    setupUBO("Matrices", UBO_MATRICES, sizeof(MatricesUBO));
    setupUBO("Data", UBO_DATA, sizeof(DataUBO));
    setupUBO("BufferInfo", UBO_BUFFER_INFO, sizeof(BufferInfoUBO));
    setupUBO("Lights", UBO_LIGHTS, sizeof(LightsUBO));
    //setupUBO("Materials", UBO_MATERIALS, sizeof(MaterialsUBO));
    setupUBO("ClipPlanes", UBO_CLIP_PLANES, sizeof(ClipPlanesUBO));
    setupUBO("Textures", UBO_TEXTURES, sizeof(TexturesUBO));

    m_sources.clear();

    return 0;
}

void Program::appendDefines(std::vector<std::string>& lines)
{
    for (const auto& [key, value] : m_defines) {
        lines.push_back(fmt::format("#define {} {}",key, value));
    }
}

void Program::setInt(const std::string& name, int value) noexcept
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform1i(vi, value);
    }
}

void Program::setupUBO(
    const char* name,
    unsigned int ubo,
    unsigned int localSize)
{
    // NOTE KI no setup really; just validation
    // => validation required to avoid serious memory corruption issues

    unsigned int blockIndex = glGetUniformBlockIndex(m_programId, name);
    if (blockIndex == GL_INVALID_INDEX) {
        KI_WARN(fmt::format(
            "PROGRAM::MISSING_UBO program={}, UBO={}",
            m_programName, name));
        return;
    }
    //glUniformBlockBinding(m_programId, blockIndex, ubo);

    GLint remoteSize;
    glGetActiveUniformBlockiv(m_programId, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &remoteSize);

    KI_INFO(fmt::format(
        "PROGRAM::UBO_SIZE program={}, UBO={}, local_size={}, remote_size={}",
        m_programName, name, localSize, remoteSize));

    if (localSize != remoteSize) {
        for (const auto& [k, v] : m_defines) {
            KI_ERROR(fmt::format("DEFINE: {}={}", k, v));
        }

        const auto msg = fmt::format(
            "PROGRAM::UBO_SIZE program={}. UBO={}. local_size={}. remote_size={}",
            m_programName, name, localSize, remoteSize);

        KI_CRITICAL(msg);
        __debugbreak();

        throw std::runtime_error{ msg };
    }
}

/**
* Load shader file
*/
std::string Program::loadSource(
    const std::string& path,
    bool optional)
{
    std::vector<std::string> lines = loadSourceLines(path, optional);

    std::stringstream sb;

    for (auto& line : lines) {
        sb << line << std::endl;
    }

    std::string src = sb.str();
    KI_TRACE(fmt::format("== {} ===\n{}", path, src));

    return src;
}

/**
* Load shader file
*/
std::vector<std::string> Program::loadSourceLines(
    const std::string& path,
    bool optional)
{
    bool exists = fileExists(path);
    if (!exists) {
        std::string msg = fmt::format("FILE_NOT_EXIST: {}", path);
        KI_INFO(msg);
        if (optional) {
            return {};
        }
        throw std::runtime_error{ msg };
    }

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
                appendDefines(lines);
                for (auto& l : processInclude(INC_GLOBALS, lineNumber)) {
                    lines.push_back(l);
                }
                lines.push_back("#line " + std::to_string(lineNumber + 1) + " " + std::to_string(lineNumber + 1));
            } else if (k == "#include") {
                for (auto& l : processInclude(v1, lineNumber)) {
                    lines.push_back(l);
                }
                lines.push_back("#line " + std::to_string(lineNumber + 1) + " " + std::to_string(lineNumber + 1));
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
                "PROGRAM::FILE_NOT_SUCCESFULLY_READ program={}, path={}",
                m_programName, path));
        }
        else {
            KI_DEBUG(fmt::format(
                "PROGRAM::FILE_NOT_SUCCESFULLY_READ program={}, path={}",
                m_programName, path));
        }
        throw;
    }

    return lines;
}

std::vector<std::string> Program::processInclude(
    const std::string& includePath,
    int lineNumber)
{
    std::string path;
    {
        std::filesystem::path fp;
        fp /= m_assets.shadersDir;
        fp /= "_" + includePath;
        path = fp.string();
    }

    std::vector<std::string> lines = loadSourceLines(path, false);

    std::vector<std::string> result;
    result.push_back("#line 1 " + std::to_string(lineNumber));
    for (auto& line : lines) {
        result.push_back(line);
    }

    return result;
}
