#include "Program.h"

#include <fstream>
#include <sstream>
#include <filesystem>

#include <fmt/format.h>

#include "util/Util.h"

#include "asset/Assets.h"

#include "pool/IdGenerator.h"

#include "kigl/GLState.h"

#include "UBO.h"
#include "MatricesUBO.h"
#include "DataUBO.h"
#include "BufferInfoUBO.h"
#include "ClipPlaneUBO.h"
#include "LightUBO.h"
#include "ProgramBind.h"
#include "Shader.h"
#include "Uniform.h"
#include "ProgramUniforms.h"

#include "ProgramRegistry.h"


namespace {
    constexpr size_t LOG_SIZE = 4096;

    const std::string GEOM_NONE{ "" };
}

Program* Program::get(ki::program_id id)
{
    return ProgramRegistry::get().getProgram(id);
}

Program::Program(
    ki::program_id id,
    std::string_view key,
    std::string_view name,
    const bool compute,
    std::string_view geometryType,
    const std::map<std::string, std::string, std::less<> >& defines)
    : m_id{ id },
    m_key{ key },
    m_programName{ name },
    m_compute{ compute },
    m_geometryType{ geometryType },
    m_defines{ defines }
{
    const auto& assets = Assets::get();

    std::string basePath;
    {
        basePath = util::joinPath(
            assets.shadersDir,
            name);
    }

    if (m_compute) {
        m_sources[GL_COMPUTE_SHADER] = { true, basePath + ".cs.glsl" };
    }
    else {
        m_sources[GL_VERTEX_SHADER] = { true, basePath + ".vs" };
        m_sources[GL_FRAGMENT_SHADER] = { true, basePath + ".fs" };
        if (geometryType.empty()) {
            m_sources[GL_GEOMETRY_SHADER] = { false, basePath + ".gs.glsl" };
        }
        else {
            m_sources[GL_GEOMETRY_SHADER] = { true, basePath + "_" + std::string{ geometryType } + ".gs.glsl" };
        }

        m_sources[GL_TESS_CONTROL_SHADER] = { false, basePath + ".tcs.glsl" };
        m_sources[GL_TESS_EVALUATION_SHADER] = { false, basePath + ".tes.glsl" };
    }
}

Program::~Program()
{
    KI_INFO(fmt::format("PROGRAM: delete - {}", m_key));
    if (m_programId >= 0) {
        glDeleteProgram(m_programId);
    }
}

void Program::bind() const noexcept
{
    assert(isReady());
    kigl::GLState::get().useProgram(m_programId);
}

bool Program::isModified() const noexcept
{
    bool modified = false;
    for (const auto& [type, source] : m_sources) {
        modified |= source.modified();
    }
    return modified;
}

void Program::load()
{
    if (m_loaded) return;

    for (auto& [type, source] : m_sources) {
        source.load(*this);
    }

    m_loaded = true;
}

void Program::reload()
{
    if (!m_loaded) return;

    try {
        m_loaded = false;
        load();
        m_prepared = false;
    }
    catch (const std::exception& ex) {
        m_loaded = true;
        KI_CRITICAL(fmt::format("PROGRAM_RELOAD: {}", ex.what()));
    }
    catch (const std::string& ex) {
        m_loaded = true;
        KI_CRITICAL(fmt::format("PROGRAM_RELOAD: {}", ex));
    }
    catch (const char* ex) {
        m_loaded = true;
        KI_CRITICAL(fmt::format("PROGRAM_RELOAD: {}", ex));
    }
    catch (...) {
        m_loaded = true;
        KI_CRITICAL("PROGRAM_RELOAD: UNKNOWN_ERROR");
    }
}

ki::program_id Program::prepareRT()
{
    if (!m_loaded) return m_id;
    if (m_prepared) return m_id;
    m_prepared = true;

    try {
        int oldProgramId = m_programId;

        createProgram();

        if (m_programId != oldProgramId) {
            if (oldProgramId != -1) {
                glDeleteProgram(oldProgramId);
            }

            m_uniforms = std::make_unique<ProgramUniforms>(*this);

            m_uniformLocations.clear();
            m_subroutineIndeces.clear();
            m_subroutineLocations.clear();
        }
    }
    catch (const std::exception& ex) {
        KI_CRITICAL(fmt::format("PROGRAM_RELOAD: {}", ex.what()));
    }
    catch (const std::string& ex) {
        KI_CRITICAL(fmt::format("PROGRAM_RELOAD: {}", ex));
    }
    catch (const char* ex) {
        KI_CRITICAL(fmt::format("PROGRAM_RELOAD: {}", ex));
    }
    catch (...) {
        KI_CRITICAL("PROGRAM_RELOAD: UNKNOWN_ERROR");
    }

    return m_id;
}

GLint Program::getUniformLoc(std::string_view name)
{
    const auto& e = m_uniformLocations.find(name);
    if (e != m_uniformLocations.end()) {
        return e->second;
    }

    std::string key{ name };
    GLint vi = glGetUniformLocation(m_programId, key.c_str());
    m_uniformLocations.insert({ key, vi });
    if (vi < 0) {
        KI_DEBUG(fmt::format(
            "PROGRAM_ERROR: MISSING_UNIFORM: {} - uniform={}",
            m_programName, name));
    }
    return vi;
}

GLint Program::getUniformSubroutineLoc(std::string_view name, GLenum shaderType)
{
    auto& map = m_subroutineLocations[shaderType];
    const auto& e = map.find(name);
    if (e != map.end()) {
        return e->second;
    }

    std::string key{ name };
    GLint vi = glGetSubroutineUniformLocation(m_programId, shaderType, key.c_str());
    map.insert({ key, vi });

    if (vi < 0) {
        KI_DEBUG(fmt::format(
            "PROGRAM_ERROR: MISSING_SUBROUTINE: {} - type={}, subroutine={}",
            m_programName, shaderType, name));
    }
    return vi;
}

GLint Program::getSubroutineIndex(std::string_view name, GLenum shaderType)
{
    auto& map = m_subroutineIndeces[shaderType];
    const auto& e = map.find(name);
    if (e != map.end()) {
        return e->second;
    }

    std::string key{ name };
    GLint vi = glGetSubroutineIndex(m_programId, shaderType, key.c_str());
    map.insert({ key, vi });

    if (vi < 0) {
        KI_DEBUG(fmt::format(
            "PROGRAM_ERROR: MISSING_SUBROUTINE: {} - type={}, subroutine={}",
            m_programName, shaderType, name));
    }
    return vi;
}

int Program::compileSource(
    GLenum shaderType,
    const ShaderSource& source)
{
    if (source.empty()) return -1;

    const char* src = source.m_source.c_str();

    const auto& shaderPath = source.m_path;
    int shaderId = glCreateShader(shaderType);

    kigl::setLabel(GL_SHADER, shaderId, shaderPath);
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
                "PROGRAM_ERROR: SHADER_COMPILE_FAILED[{:#04x}] PROGRAM={}\nPATH={}\n{}",
                shaderType, m_programName, shaderPath, infoLog));
            KI_ERROR(fmt::format(
                "FAILED_SOURCE:\n-------------------\n{}\n-------------------",
                util::appendLineNumbers(source.m_source)));

            glDeleteShader(shaderId);
            shaderId = -1;

            const auto msg = fmt::format(
                "PROGRAM_ERROR: SHADER_COMPILE_FAILED[{:#04x}] PROGRAM={}, PATH={}",
                shaderType, m_programName, shaderPath);
            throw std::runtime_error{ msg };
        }
    }

    return shaderId;
}

void Program::createProgram() {
    KI_INFO_OUT(fmt::format("[PROGRAM_CREATE - {}]", m_key));

    // build and compile our shader program
    // ------------------------------------
    std::unordered_map<GLenum, int> shaderIds;
    for (auto& [type, source] : m_sources) {
        shaderIds[type] = compileSource(type, m_sources[type]);
    }

    // link shaders
    int programId = -1;
    {
        programId = glCreateProgram();

        kigl::setLabel(GL_PROGRAM, programId, m_key);

        for (auto& [type, shaderId] : shaderIds) {
            if (shaderId == -1) continue;
            glAttachShader(programId, shaderId);
            kigl::setLabel(GL_SHADER, shaderId, m_sources[type].m_path);
        }

        glLinkProgram(programId);

        // check for linking errors
        {
            int success;
            glGetProgramiv(programId, GL_LINK_STATUS, &success);
            if (!success) {
                char infoLog[LOG_SIZE];
                glGetProgramInfoLog(programId, LOG_SIZE, NULL, infoLog);
                const auto msg = fmt::format(
                    "PROGRAM_ERROR: PROGRAM::LINKING_FAILED program={}\n{}",
                    m_programName, infoLog);
                KI_ERROR(msg);

                glDeleteProgram(programId);
                programId = -1;

                throw std::runtime_error{ msg };
            }
        }

        for (auto& [type, shaderId] : shaderIds) {
            if (shaderId == -1) continue;
            glDeleteShader(shaderId);
        }
    }

    for (auto& [type, source] : m_sources) {
        source.clear();
    }

    if (programId != -1) {
        validateProgram(programId);
        initProgram(programId);

        m_programId = programId;
    }
}

// https://community.khronos.org/t/samplers-of-different-types-use-the-same-textur/66329/4
void Program::validateProgram(int programId) const {
    if (programId == -1) return;
    glValidateProgram(programId);

    int success;
    glGetProgramiv(programId, GL_VALIDATE_STATUS, &success);
    if (!success) {
        char infoLog[LOG_SIZE];
        glGetProgramInfoLog(programId, LOG_SIZE, NULL, infoLog);

        const auto msg = fmt::format(
            "PROGRAM_ERROR: PROGRAM::VALIDATE_FAILED program={}\n{}",
            m_programName, infoLog);

        KI_ERROR(msg);

        throw std::runtime_error{ msg };
    }
}

void Program::initProgram(int programId) const
{
    KI_INFO_OUT(fmt::format("[PROGRAM - {}]", m_key));

    // NOTE KI set UBOs only once for program
    validateUBO(programId, "Matrices", UBO_MATRICES, sizeof(MatricesUBO));
    validateUBO(programId, "Data", UBO_DATA, sizeof(DataUBO));
    validateUBO(programId, "BufferInfo", UBO_BUFFER_INFO, sizeof(BufferInfoUBO));
    validateUBO(programId, "Lights", UBO_LIGHTS, sizeof(LightsUBO));
    //validateUBO("Materials", UBO_MATERIALS, sizeof(MaterialsUBO));
    validateUBO(programId, "ClipPlanes", UBO_CLIP_PLANES, sizeof(ClipPlanesUBO));
    //validateUBO("Textures", UBO_TEXTURES, sizeof(TexturesUBO));
}

void Program::validateUBO(
    int programId,
    const char* name,
    unsigned int ubo,
    unsigned int localSize) const
{
    // NOTE KI no setup really; just validation
    // => validation required to avoid serious memory corruption issues

    unsigned int blockIndex = glGetUniformBlockIndex(programId, name);
    if (blockIndex == GL_INVALID_INDEX) {
        KI_WARN(fmt::format(
            "PROGRAM_ERROR: MISSING_UBO program={}, UBO={}",
            m_programName, name));
        return;
    }
    //glUniformBlockBinding(programId, blockIndex, ubo);

    GLint remoteSize;
    glGetActiveUniformBlockiv(programId, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &remoteSize);

    KI_INFO(fmt::format(
        "PROGRAM_ERROR: UBO_SIZE program={}, UBO={}, local_size={}, remote_size={}",
        m_programName, name, localSize, remoteSize));

    if (localSize != remoteSize) {
        for (const auto& [k, v] : m_defines) {
            KI_ERROR(fmt::format("DEFINE: {}={}", k, v));
        }

        const auto msg = fmt::format(
            "PROGRAM_ERROR: UBO_SIZE program={}. UBO={}. local_size={}. remote_size={}",
            m_programName, name, localSize, remoteSize);

        KI_CRITICAL(msg);

        throw std::runtime_error{ msg };
    }
}

void Program::setInt(std::string_view name, int value) noexcept
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform1i(vi, value);
    }
}

void Program::setFloat(std::string_view name, float value) noexcept
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform1f(vi, value);
    }
}

void Program::setMat4(std::string_view name, const glm::mat4& value) noexcept
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniformMatrix4fv(vi, 1, GL_FALSE, glm::value_ptr(value));
    }
}

