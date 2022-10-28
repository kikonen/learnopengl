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
    const std::map<std::string, std::string>& defines)
    : objectID(nextID()),
    assets(assets),
    key(key),
    shaderName(name),
    geometryType(geometryType),
    defines(defines)
{
    std::string basePath;
    {
        std::filesystem::path fp;
        fp /= assets.shadersDir;
        fp /= name;
        basePath = fp.string();
    }

    paths[GL_VERTEX_SHADER] = basePath + ".vs";
    paths[GL_FRAGMENT_SHADER] = basePath + ".fs";
    paths[GL_GEOMETRY_SHADER] = basePath + geometryType + ".gs.glsl";

    //paths[GL_TESS_CONTROL_SHADER] = basePath + ".tcs.glsl";
    //paths[GL_TESS_EVALUATION_SHADER] = basePath + ".tes.glsl";

    required[GL_VERTEX_SHADER] = true;
    required[GL_FRAGMENT_SHADER] = true;
    required[GL_GEOMETRY_SHADER] = !geometryType.empty();
    required[GL_TESS_CONTROL_SHADER] = false;
    required[GL_TESS_EVALUATION_SHADER] = false;

    bindTexture = true;
}

Shader::~Shader()
{
    KI_INFO_SB("DELETE: shader " << shaderName);
    if (programId != -1) {
        glDeleteProgram(programId);
    }
}

const void Shader::bind()
{
    m_bound++;
    if (m_bound > 1) return;
    assert(m_prepared);
    KI_GL_CALL(glUseProgram(programId));
}

const void Shader::unbind()
{
    m_bound--;
    assert(m_bound >= 0);
    // NOTE KI not really need to unbind program
    //if (m_bound == 0) glUseProgram(0);
}

void Shader::load()
{
    for (auto& [type, path] : paths) {
        sources[type] = loadSource(path, !required[type]);
    }
}

int Shader::prepare(const Assets& assets)
{
    if (m_prepared) return m_prepareResult;
    m_prepared = true;

    if (createProgram()) {
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
    const auto& e = uniformLocations.find(name);
    if (e != uniformLocations.end()) {
        return e->second;
    }

    GLint vi = glGetUniformLocation(programId, name.c_str());
    uniformLocations[name] = vi;
    if (vi < 0) {
        KI_WARN_SB("SHADER::MISSING_UNIFORM: " << shaderName << " uniform=" << name);
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
            KI_ERROR_SB("SHADER::GEOMETRY::COMPILATION_FAILED " << shaderName << " frag=" << shaderPath << "\n" << infoLog);
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
    for (auto& [type, source] : sources) {
        shaderIds[type] = compileSource(type, paths[type], source);
    }

    // link shaders
    {
        programId = glCreateProgram();

        for (auto& [type, shaderId] : shaderIds) {
            if (shaderId == -1) continue;
            glAttachShader(programId, shaderId);
        }

        glLinkProgram(programId);

        // check for linking errors
        {
            int success;
            glGetProgramiv(programId, GL_LINK_STATUS, &success);
            if (!success) {
                char infoLog[512];
                glGetProgramInfoLog(programId, 512, NULL, infoLog);
                KI_ERROR_SB("SHADER::PROGRAM::LINKING_FAILED " << shaderName << "\n" << infoLog);
                KI_BREAK();

                glDeleteProgram(programId);
                programId = -1;
            }
        }

        for (auto& [type, shaderId] : shaderIds) {
            if (shaderId == -1) continue;
            glDeleteShader(shaderId);
        }
    }

    if (programId == -1) return -1;

    initProgram();
    return 0;
}

// https://community.khronos.org/t/samplers-of-different-types-use-the-same-textur/66329/4
void Shader::validateProgram() {
    if (programId == -1) return;
    glValidateProgram(programId);

    int success;
    glGetProgramiv(programId, GL_VALIDATE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(programId, 512, NULL, infoLog);
        KI_ERROR_SB("SHADER::PROGRAM::VALIDATE_FAILED " << shaderName << "\n" << infoLog);
        KI_BREAK();
    }
}

int Shader::initProgram() {
    // NOTE KI set UBOs only once for shader
    setUBO("Matrices", UBO_MATRICES, sizeof(MatricesUBO));
    setUBO("Data", UBO_DATA, sizeof(DataUBO));
    setUBO("Lights", UBO_LIGHTS, sizeof(LightsUBO));
    setUBO("Materials", UBO_MATERIALS, sizeof(MaterialsUBO));
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
    normalMap.init(this); 

    //drawInstanced.init(*this);

    effect.init(this);

    nearPlane.init(this); 
    farPlane.init(this);

    skybox.init(this);

    viewportTexture.init(this);

    //prepareTextureUniform();
    prepareTextureUniforms();

    sources.clear();

    return 0;
}

void Shader::appendDefines(std::vector<std::string>& lines)
{
    for (const auto& [key, value] : defines) {
        lines.push_back(fmt::format("#define {} {}, ",key, value));
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

    char name[16];
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        sprintf_s(name, "textures[%i]", i);
        Shader::Int& v = textures.emplace_back(name);
        v.init(this);
    }
}

void Shader::setInt(const std::string& name, int value)
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform1i(vi, value);
    }
}

void Shader::setUBO(const std::string& name, unsigned int UBO, unsigned int expectedSize)
{
    unsigned int blockIndex = glGetUniformBlockIndex(programId, name.c_str());
    if (blockIndex == GL_INVALID_INDEX) {
        KI_WARN_SB("SHADER::MISSING_UBO " << shaderName << " UBO=" << name);
        return;
    } 
    glUniformBlockBinding(programId, blockIndex, UBO);

    GLint  blockSize;
    glGetActiveUniformBlockiv(programId, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    KI_INFO_SB("SHADER::UBO_SIZE " << shaderName << " UBO=" << name << " size=" << blockSize << " expected_size=" << expectedSize);
    if (blockSize != expectedSize) {
        KI_CRITICAL_SB("SHADER::UBO_SIZE " << shaderName << " UBO=" << name << " size=" << blockSize << " expected_size=" << expectedSize);
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
            KI_ERROR_SB("SHADER::FILE_NOT_SUCCESFULLY_READ " << shaderName << " path=" << path);
            KI_BREAK();
        }
        else {
            KI_DEBUG_SB("SHADER::FILE_NOT_SUCCESFULLY_READ " << shaderName << " path=" << path);
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

