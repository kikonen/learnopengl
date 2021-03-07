#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>

#include "UBO.h"

namespace {
    // name + geom
    std::map<std::string, Shader*> shaders;

    std::mutex shaders_lock;
}

Shader* Shader::getShader(
    const Assets& assets,
    const std::string& name)
{
    return Shader::getShader(assets, name, "");
}

Shader* Shader::getShader(
    const Assets& assets, 
    const std::string& name, 
    const std::string& geometryType)
{
    std::lock_guard<std::mutex> lock(shaders_lock);

    std::string key = name + geometryType;
    Shader* shader = shaders[key];

    if (!shader) {
        shader = new Shader(assets, key, name, geometryType);
        shaders[key] = shader;
        shader->load();
    }

    return shader;
}

Shader::Shader(
    const Assets& assets,
    const std::string& key,
    const std::string& name,
    const std::string& geometryType)
    : assets(assets),
    key(key),
    shaderName(name),
    geometryType(geometryType),
    geometryOptional(geometryType.empty())
{
    std::string basePath = assets.shadersDir + "/" + name;
    vertexShaderPath = basePath + ".vs";
    fragmentShaderPath = basePath + ".fs";
    geometryShaderPath = basePath + geometryType + ".gs";

    bindTexture = true;
}

Shader::~Shader()
{
    glDeleteProgram(programId);
}

const void Shader::bind()
{
    KI_GL_CALL(glUseProgram(programId));
}

const void Shader::unbind()
{
    glUseProgram(0);
}

void Shader::load()
{
    vertexShaderSource = loadSource(vertexShaderPath, false);
    fragmentShaderSource = loadSource(fragmentShaderPath, false);
    geometryShaderSource = loadSource(geometryShaderPath, geometryOptional);
}

int Shader::prepare()
{
    if (prepared) {
        return res;
    }
    prepared = true;
    res = -1;

    if (createProgram()) {
        return -1;
    }
    res = 0;
    return res;
}

GLint Shader::getUniformLoc(const std::string& name)
{
    if (uniformLocations.count(name)) {
        return uniformLocations[name];
    }

    GLint vi = glGetUniformLocation(programId, name.c_str());
    uniformLocations[name] = vi;
    if (vi < 0) {
        KI_WARN_SB("SHADER::MISSING_UNIFORM: " << shaderName << " uniform=" << name);
    }
    return vi;
}

int Shader::createProgram() {
    int success;
    char infoLog[512];

    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        KI_ERROR_SB("SHADER::FILE_EMPTY " << shaderName);
        KI_BREAK();
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    {
        const char* src = vertexShaderSource.c_str();
        glShaderSource(vertexShader, 1, &src, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            KI_ERROR_SB("SHADER::VERTEX::COMPILATION_FAILED " << shaderName << " vert=" << vertexShaderPath << "\n" << infoLog);
            KI_BREAK();
        }
    }

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    {
        const char* src = fragmentShaderSource.c_str();
        glShaderSource(fragmentShader, 1, &src, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            KI_ERROR_SB("SHADER::FRAGMENT::COMPILATION_FAILED " << shaderName << " frag=" << fragmentShaderPath << "\n" << infoLog);
            KI_BREAK();
        }
    }

    // geoemtry shader
    int geometryShader = -1;
    if (!geometryShaderSource.empty()) {
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);

        const char* src = geometryShaderSource.c_str();
        glShaderSource(geometryShader, 1, &src, NULL);
        glCompileShader(geometryShader);
        // check for shader compile errors
        glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
            KI_ERROR_SB("SHADER::GEOMETRY::COMPILATION_FAILED " << shaderName << " frag=" << geometryShaderPath << "\n" << infoLog);
            KI_BREAK();
        }
    }

    // link shaders
    programId = glCreateProgram();

    glAttachShader(programId, vertexShader);
    glAttachShader(programId, fragmentShader);
    if (geometryShader != -1) {
        glAttachShader(programId, geometryShader);
    }

    glLinkProgram(programId);

    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programId, 512, NULL, infoLog);
        KI_ERROR_SB("SHADER::PROGRAM::LINKING_FAILED " << shaderName << "\n" << infoLog);
        KI_BREAK();
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryShader != -1) {
        glDeleteShader(geometryShader);
    }

    glValidateProgram(programId);
    if (!success) {
        glGetProgramInfoLog(programId, 512, NULL, infoLog);
        KI_ERROR_SB("SHADER::PROGRAM::VALIDATE_FAILED " << shaderName << "\n" << infoLog);
        KI_BREAK();
    }

    // NOTE KI set UBOs only once for shader
    setUBO("Matrices", UBO_MATRICES);
    setUBO("Data", UBO_DATA);
    setUBO("Lights", UBO_LIGHTS);
    setUBO("Materials", UBO_MATERIALS);
    setUBO("ClipPlanes", UBO_CLIP_PLANES);

    projectionMatrix.init(this);
    viewMatrix.init(this);
    //modelMatrix.init(this); 
    //normalMatrix.init(this);

    noiseTex.init(this);
    reflectionTex.init(this);
    refractionTex.init(this);

    reflectionMap.init(this);
    refractionMap.init(this);

    shadowMap.init(this);
    normalMap.init(this); 

    //drawInstanced.init(this);

    effect.init(this);

    nearPlane.init(this); 
    farPlane.init(this);

    skybox.init(this);

    viewportTexture.init(this);

    //prepareTextureUniform();
    prepareTextureUniforms();

    vertexShaderSource = "";
    fragmentShaderSource = "";
    geometryShaderSource = "";

    return 0;
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

void Shader::setUBO(const std::string& name, unsigned int UBO)
{
    unsigned int blockIndex = glGetUniformBlockIndex(programId, name.c_str());
    if (blockIndex == GL_INVALID_INDEX) {
        KI_WARN_SB("SHADER::MISSING_UBO " << shaderName << " UBO=" << name);
        return;
    } 
    glUniformBlockBinding(programId, blockIndex, UBO);
}

/**
* Load shader file
*/
std::string Shader::loadSource(const std::string& path, bool optional) {
    std::vector<std::string> lines = loadSourceLines(path, optional);

    std::stringstream sb;
    
    for (auto line : lines) {
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
    std::ifstream file;

    std::vector<std::string> lines;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        std::ifstream file;
        //	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
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

            if (k == "#include") {
                for (auto l : processInclude(v1, lineNumber)) {
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
            KI_INFO_SB("SHADER::FILE_NOT_SUCCESFULLY_READ " << shaderName << " path=" << path);
        }
    }
    KI_INFO_SB("FILE: " << path);

    return lines;
}

std::vector<std::string> Shader::processInclude(const std::string& includePath, int lineNumber) 
{
    std::string path = assets.shadersDir + "/_" + includePath;
    std::vector<std::string> lines = loadSourceLines(path, false);

    std::vector<std::string> result;
    result.push_back("#line 1 " + std::to_string(lineNumber));
    for (auto line : lines) {
        result.push_back(line);
    }

    return result;
}

