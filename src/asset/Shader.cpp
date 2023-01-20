#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>
#include <filesystem>

#include <fmt/format.h>

#include "UBO.h"

#include "asset/MatricesUBO.h"
#include "asset/DataUBO.h"
#include "asset/ClipPlaneUBO.h"
#include "asset/LightUBO.h"
#include "asset/TextureUBO.h"

#include "ShaderBind.h"

namespace {
    constexpr int LOG_SIZE = 4096;

    int idBase = 0;

    std::mutex type_id_lock;

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

Shader::Shader(
    const Assets& assets,
    const std::string& key,
    const std::string& name,
    const bool compute,
    const std::string& geometryType,
    const std::map<std::string, std::string>& defines)
    : m_objectID(nextID()),
    assets(assets),
    m_key(key),
    m_shaderName(name),
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

        //paths[GL_TESS_CONTROL_SHADER] = basePath + ".tcs.glsl";
        //paths[GL_TESS_EVALUATION_SHADER] = basePath + ".tes.glsl";

        m_required[GL_VERTEX_SHADER] = true;
        m_required[GL_FRAGMENT_SHADER] = true;
        m_required[GL_GEOMETRY_SHADER] = !geometryType.empty();
        m_required[GL_TESS_CONTROL_SHADER] = false;
        m_required[GL_TESS_EVALUATION_SHADER] = false;

        //m_bindTexture = true;
    }
}

Shader::~Shader()
{
    KI_INFO(fmt::format("DELETE: shader={}", m_key));
    if (m_programId != -1) {
        glDeleteProgram(m_programId);
    }
}

void Shader::bind(GLState& state) const noexcept
{
    assert(m_prepared);
    state.useProgram(m_programId);
}

void Shader::load()
{
    for (auto& [type, path] : m_paths) {
        m_sources[type] = loadSource(path, !m_required[type]);
    }
}

int Shader::prepare(const Assets& assets) noexcept
{
    if (m_prepared) return m_prepareResult;
    m_prepared = true;

    if (createProgram()) {
        m_prepareResult = -1;
        return -1;
    }

    m_prepareResult = 0;
    return m_prepareResult;
}

GLint Shader::getUniformLoc(const std::string& name)
{
    const auto& e = m_uniformLocations.find(name);
    if (e != m_uniformLocations.end()) {
        return e->second;
    }

    GLint vi = glGetUniformLocation(m_programId, name.c_str());
    m_uniformLocations[name] = vi;
    if (vi < 0) {
        KI_DEBUG(fmt::format(
            "SHADER::MISSING_UNIFORM: {} - uniform={}",
            m_shaderName, name));
    }
    return vi;
}

GLuint Shader::getUniformSubroutineLoc(const std::string& name, GLenum shaderType)
{
    auto& map = m_subroutineLocations[shaderType];
    const auto& e = map.find(name);
    if (e != map.end()) {
        return e->second;
    }

    GLuint vi = glGetSubroutineUniformLocation(m_programId, shaderType, name.c_str());
    map[name] = vi;
    if (vi < 0) {
        KI_DEBUG(fmt::format(
            "SHADER::MISSING_SUBROUTINE: {} - type={}, subroutine={}",
            m_shaderName, shaderType, name));
    }
    return vi;
}

GLuint Shader::getSubroutineIndex(const std::string& name, GLenum shaderType)
{
    auto& map = m_subroutineIndeces[shaderType];
    const auto& e = map.find(name);
    if (e != map.end()) {
        return e->second;
    }

    GLuint vi = glGetSubroutineIndex(m_programId, shaderType, name.c_str());
    map[name] = vi;
    if (vi < 0) {
        KI_DEBUG(fmt::format(
            "SHADER::MISSING_SUBROUTINE: {} - type={}, subroutine={}",
            m_shaderName, shaderType, name));
    }
    return vi;
}

int Shader::compileSource(
    GLenum shaderType,
    const std::string& shaderPath,
    const std::string& source)
{
    if (source.empty()) return -1;

    const char* src = source.c_str();

    int shaderId = glCreateShader(shaderType);
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
                "SHADER:::COMPILE_FAILED[{:#04x}] SHADER={}\nPATH={}\n{}",
                shaderType, m_shaderName, shaderPath, infoLog));
            KI_ERROR(fmt::format(
                "FAILED_SOURCE:\n-------------------\n{}\n-------------------",
                source));
            KI_BREAK();

            glDeleteShader(shaderId);
            shaderId = -1;
        }
    }

    return shaderId;
}

int Shader::createProgram() {
    // build and compile our shader program
    // ------------------------------------
    std::map<GLenum, GLuint> shaderIds;
    for (auto& [type, source] : m_sources) {
        shaderIds[type] = compileSource(type, m_paths[type], source);
    }

    // link shaders
    {
        m_programId = glCreateProgram();

        for (auto& [type, shaderId] : shaderIds) {
            if (shaderId == -1) continue;
            glAttachShader(m_programId, shaderId);
        }

        glLinkProgram(m_programId);

        // check for linking errors
        {
            int success;
            glGetProgramiv(m_programId, GL_LINK_STATUS, &success);
            if (!success) {
                char infoLog[LOG_SIZE];
                glGetProgramInfoLog(m_programId, LOG_SIZE, NULL, infoLog);
                KI_ERROR(fmt::format(
                    "SHADER::PROGRAM::LINKING_FAILED shader={}\n{}",
                    m_shaderName, infoLog));
                KI_BREAK();

                glDeleteProgram(m_programId);
                m_programId = -1;
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
void Shader::validateProgram() const {
    if (m_programId == -1) return;
    glValidateProgram(m_programId);

    int success;
    glGetProgramiv(m_programId, GL_VALIDATE_STATUS, &success);
    if (!success) {
        char infoLog[LOG_SIZE];
        glGetProgramInfoLog(m_programId, LOG_SIZE, NULL, infoLog);
        KI_ERROR(fmt::format(
            "SHADER::PROGRAM::VALIDATE_FAILED shader={}\n{}",
            m_shaderName, infoLog));
        KI_BREAK();
    }
}

int Shader::initProgram() {
    KI_INFO_OUT(fmt::format("[SHADER - {}]", m_key));

#ifdef _DEBUG
    // NOTE KI set UBOs only once for shader
    setupUBO("Matrices", UBO_MATRICES, sizeof(MatricesUBO));
    setupUBO("Data", UBO_DATA, sizeof(DataUBO));
    setupUBO("Lights", UBO_LIGHTS, sizeof(LightsUBO));
    //setupUBO("Materials", UBO_MATERIALS, sizeof(MaterialsUBO));
    setupUBO("ClipPlanes", UBO_CLIP_PLANES, sizeof(ClipPlanesUBO));
    setupUBO("Textures", UBO_TEXTURES, sizeof(TexturesUBO));
#endif

    u_projectionMatrix.init(this);
    u_viewMatrix.init(this);
    //u_modelMatrix.init(this);
    //u_normalMatrix.init(this);

    //u_noiseTex.init(this);
    //u_reflectionTex.init(this);
    //vrefractionTex.init(this);

    //u_cubeMap.init(this);

    //u_shadowMap.init(this);
    //u_normalMap.init(this);

    //u_drawInstanced.init(*this);

    u_effect.init(this);

    u_nearPlane.init(this);
    u_farPlane.init(this);

    //u_skybox.init(this);

    //u_viewportTex.init(this);

    m_sources.clear();

    return 0;
}

void Shader::appendDefines(std::vector<std::string>& lines)
{
    for (const auto& [key, value] : m_defines) {
        lines.push_back(fmt::format("#define {} {}",key, value));
    }
}

//void Shader::prepareTextureUniforms()
//{
//    textures.reserve(TEXTURE_COUNT);
//
//    for (int i = 0; i < TEXTURE_COUNT; i++) {
//        auto name = fmt::format("u_textures[{}]", i);
//        Shader::Int& v = textures.emplace_back(name);
//        v.init(this);
//    }
//}

void Shader::setInt(const std::string& name, int value) noexcept
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform1i(vi, value);
    }
}

void Shader::setupUBO(
    const char* name,
    unsigned int ubo,
    unsigned int expectedSize) noexcept
{
    // NOTE KI no setup really; just validation
    // => validation required to avoid serious memory corruption issues

    unsigned int blockIndex = glGetUniformBlockIndex(m_programId, name);
    if (blockIndex == GL_INVALID_INDEX) {
        KI_WARN(fmt::format(
            "SHADER::MISSING_UBO shader={}, UBO={}",
            m_shaderName, name));
        return;
    }
    //glUniformBlockBinding(m_programId, blockIndex, ubo);

    GLint blockSize;
    glGetActiveUniformBlockiv(m_programId, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    KI_INFO(fmt::format(
        "SHADER::UBO_SIZE shader={}, UBO={}, size={}, expected_size={}",
        m_shaderName, name, blockSize, expectedSize));

    if (blockSize != expectedSize) {
        for (const auto& [k, v] : m_defines) {
            KI_ERROR(fmt::format("DEFINE: {}={}", k, v));
        }
        KI_CRITICAL(fmt::format(
            "SHADER::UBO_SIZE shader={}. UBO={}. size={}. expected_size={}",
            m_shaderName, name, blockSize, expectedSize));
        __debugbreak();
    }
}

/**
* Load shader file
*/
std::string Shader::loadSource(
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
std::vector<std::string> Shader::loadSourceLines(
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
                "SHADER::FILE_NOT_SUCCESFULLY_READ shader={}, path={}",
                m_shaderName, path));
            KI_BREAK();
        }
        else {
            KI_DEBUG(fmt::format(
                "SHADER::FILE_NOT_SUCCESFULLY_READ shader={}, path={}",
                m_shaderName, path));
        }
    }

    return lines;
}

std::vector<std::string> Shader::processInclude(
    const std::string& includePath,
    int lineNumber)
{
    std::string path;
    {
        std::filesystem::path fp;
        fp /= assets.shadersDir;
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

