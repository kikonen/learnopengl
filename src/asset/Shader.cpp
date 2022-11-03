#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>
#include <filesystem>

#include <fmt/format.h>

#include "UBO.h"

#include "ShaderBind.h"

namespace {
    int typeIDbase = 0;

    std::mutex type_id_lock;

    int nextID()
    {
        std::lock_guard<std::mutex> lock(type_id_lock);
        return ++typeIDbase;
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
    const std::string& geometryType,
    const int materialCount,
    const std::map<std::string, std::string>& defines)
    : m_objectID(nextID()),
    assets(assets),
    m_key(key),
    m_shaderName(name),
    m_geometryType(geometryType),
    m_materialCount(materialCount),
    m_defines(defines)
{
    m_defines[DEF_MAT_COUNT] = std::to_string(materialCount);

    std::string basePath;
    {
        std::filesystem::path fp;
        fp /= assets.shadersDir;
        fp /= name;
        basePath = fp.string();
    }

    m_paths[GL_VERTEX_SHADER] = basePath + ".vs";
    m_paths[GL_FRAGMENT_SHADER] = basePath + ".fs";
    m_paths[GL_GEOMETRY_SHADER] = basePath + geometryType + ".gs.glsl";

    //paths[GL_TESS_CONTROL_SHADER] = basePath + ".tcs.glsl";
    //paths[GL_TESS_EVALUATION_SHADER] = basePath + ".tes.glsl";

    m_required[GL_VERTEX_SHADER] = true;
    m_required[GL_FRAGMENT_SHADER] = true;
    m_required[GL_GEOMETRY_SHADER] = !geometryType.empty();
    m_required[GL_TESS_CONTROL_SHADER] = false;
    m_required[GL_TESS_EVALUATION_SHADER] = false;

    //m_bindTexture = true;
}

Shader::~Shader()
{
    KI_INFO_SB("DELETE: shader " << m_shaderName);
    if (m_programId != -1) {
        glDeleteProgram(m_programId);
    }
}

const void Shader::bind() noexcept
{
    m_bound++;
    if (m_bound > 1) return;
    assert(m_prepared);
    KI_GL_CALL(glUseProgram(m_programId));
}

const void Shader::unbind() noexcept
{
    m_bound--;
    assert(m_bound >= 0);
    // NOTE KI not really need to unbind program
    //if (m_bound == 0) glUseProgram(0);
}

void Shader::load()
{
    for (auto& [type, path] : m_paths) {
        m_sources[type] = loadSource(path, !m_required[type]);
    }
}

int Shader::prepare(const Assets& assets) noexcept
{
    if (m_shaderName == TEX_TEXTURE && m_materialCount == 0)
        KI_BREAK();
    if (m_prepared) return m_prepareResult;
    m_prepared = true;

    if (createProgram()) {
        m_prepareResult = -1;
        return -1;
    }

    {
        ShaderBind bound(this);

        cubeMap.set(assets.cubeMapUnitIndex);
        shadowMap.set(assets.shadowMapUnitIndex);
        skybox.set(assets.skyboxUnitIndex);
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
        KI_WARN_SB("SHADER::MISSING_UNIFORM: " << m_shaderName << " uniform=" << name);
        vi = -1;
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
            char infoLog[512];
            glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
            KI_ERROR_SB("SHADER::GEOMETRY::COMPILATION_FAILED " << m_shaderName << " frag=" << shaderPath << "\n" << infoLog);
            KI_ERROR_SB(source);
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
                char infoLog[512];
                glGetProgramInfoLog(m_programId, 512, NULL, infoLog);
                KI_ERROR_SB("SHADER::PROGRAM::LINKING_FAILED " << m_shaderName << "\n" << infoLog);
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
void Shader::validateProgram() {
    if (m_programId == -1) return;
    glValidateProgram(m_programId);

    int success;
    glGetProgramiv(m_programId, GL_VALIDATE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(m_programId, 512, NULL, infoLog);
        KI_ERROR_SB("SHADER::PROGRAM::VALIDATE_FAILED " << m_shaderName << "\n" << infoLog);
        KI_BREAK();
    }
}

int Shader::initProgram() {
    std::cout << "[SHADER - " << m_key << "]\n";

    // NOTE KI set UBOs only once for shader
    setUBO("Matrices", UBO_MATRICES, sizeof(MatricesUBO));
    setUBO("Data", UBO_DATA, sizeof(DataUBO));
    setUBO("Lights", UBO_LIGHTS, sizeof(LightsUBO));
    if (m_materialCount > 0) {
        setUBO("Materials", UBO_MATERIALS, m_materialCount * sizeof(MaterialUBO));
    }
    setUBO("ClipPlanes", UBO_CLIP_PLANES, sizeof(ClipPlanesUBO));


    projectionMatrix.init(this);
    viewMatrix.init(this);
    //modelMatrix.init(this); 
    //normalMatrix.init(this);

    noiseTex.init(this);
    reflectionTex.init(this);
    refractionTex.init(this);

    cubeMap.init(this);

    shadowMap.init(this);
    //normalMap.init(this); 

    //drawInstanced.init(*this);

    effect.init(this);

    nearPlane.init(this); 
    farPlane.init(this);

    skybox.init(this);

    viewportTex.init(this);

    //prepareTextureUniform();
    prepareTextureUniforms();

    m_sources.clear();

    return 0;
}

void Shader::appendDefines(std::vector<std::string>& lines)
{
    for (const auto& [key, value] : m_defines) {
        lines.push_back(fmt::format("#define {} {}",key, value));
    }
}

//void Shader::prepareTextureUniform()
//{
//    TextureInfo* info = new TextureInfo();
//    info->diffuseTex = new Shader::Int("diffuse");
//    info->diffuseTex->init(this);
//
//    info->emissionTex = new Shader::Int("emission");
//    info->emissionTex->init(this);
//
//    info->specularTex = new Shader::Int("specular");
//    info->specularTex->init(this);
//
//    info->normalMap = new Shader::Int("normalMap");
//    info->normalMap->init(this);
//
//    texture = info;
//}

void Shader::prepareTextureUniforms()
{
    textures.reserve(TEXTURE_COUNT);

    for (int i = 0; i < TEXTURE_COUNT; i++) {
        auto name = fmt::format("u_textures[{}]", i);
        Shader::Int& v = textures.emplace_back(name);
        v.init(this);
    }
}

void Shader::setInt(const std::string& name, int value) noexcept
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform1i(vi, value);
    }
}

void Shader::setUBO(
    const std::string& name,
    unsigned int UBO,
    unsigned int expectedSize) noexcept
{
    unsigned int blockIndex = glGetUniformBlockIndex(m_programId, name.c_str());
    if (blockIndex == GL_INVALID_INDEX) {
        KI_WARN_SB("SHADER::MISSING_UBO " << m_shaderName << " UBO=" << name);
        return;
    } 
    glUniformBlockBinding(m_programId, blockIndex, UBO);

    GLint blockSize;
    glGetActiveUniformBlockiv(m_programId, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    KI_INFO_SB("SHADER::UBO_SIZE " << m_shaderName << " UBO=" << name << " size=" << blockSize << " expected_size=" << expectedSize);
    if (blockSize != expectedSize) {
        for (const auto& [k, v] : m_defines) {
            KI_ERROR_SB(k << "=" << v);
        }
        KI_ERROR_SB(fmt::format("materialCount={}, sizeof(MaterialUBO)=", m_materialCount, sizeof(MaterialUBO)));
        KI_CRITICAL_SB(fmt::format(
            "SHADER::UBO_SIZE shader={}. UBO={}. size={}. expected_size={}",
            m_shaderName, name, blockSize, expectedSize));
        __debugbreak();
    }
}

/**
* Load shader file
*/
std::string Shader::loadSource(const std::string& path, bool optional) {
    std::vector<std::string> lines = loadSourceLines(path, optional);

    std::stringstream sb;
    
    for (auto& line : lines) {
        sb << line << std::endl;
    }

    std::string src = sb.str();
    KI_TRACE_SB("== " << path << " ===\n" << src);

    return src;
}

/**
* Load shader file
*/
std::vector<std::string> Shader::loadSourceLines(const std::string& path, bool optional) {
    bool exists = fileExists(path);
    if (!exists && optional) return {};

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

        file.close();
    }
    catch (std::ifstream::failure e) {
        if (!optional) {
            KI_ERROR_SB("SHADER::FILE_NOT_SUCCESFULLY_READ " << m_shaderName << " path=" << path);
            KI_BREAK();
        }
        else {
            KI_DEBUG_SB("SHADER::FILE_NOT_SUCCESFULLY_READ " << m_shaderName << " path=" << path);
        }
    }
    KI_INFO_SB("FILE: " << path);

    return lines;
}

std::vector<std::string> Shader::processInclude(const std::string& includePath, int lineNumber) 
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

