#include "Shader.h"

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "UBO.h"

// name + key = tex + geom
std::map<std::string, Shader*> shaders;


Shader* Shader::getShader(
    const Assets& assets, 
    const std::string& name, 
    const std::string& textureType,
    const std::string& geometryType)
{
    std::string key = name + "_" + textureType + "_" + geometryType;
    Shader* shader = shaders[key];

    if (!shader) {
        shader = new Shader(assets, name, textureType, geometryType);
        shaders[name] = shader;
    }

    return shader;
}

Shader::Shader(
    const Assets& assets,
    const std::string& name,
    const std::string& textureType,
    const std::string& geometryType)
    : assets(assets),
    shaderName(name),
    textureType(textureType),
    geometryType(geometryType),
    geometryOptional(geometryType.empty())
{
    std::string basePath = assets.shadersDir + "/" + name;
    vertexShaderPath = basePath + textureType + ".vs";
    fragmentShaderPath = basePath + textureType + ".fs";
    geometryShaderPath = basePath + textureType + geometryType + ".gs";

    bindTexture = true;
}

Shader::~Shader()
{
    glDeleteProgram(id);
    id = 0;
}

const void Shader::use()
{
    glUseProgram(id); 
}

int Shader::setup()
{
    if (setupDone) {
        return res;
    }
    setupDone = true;
    res = -1;

    vertexShaderSource = loadSource(vertexShaderPath, false);
    fragmentShaderSource = loadSource(fragmentShaderPath, false);
    geometryShaderSource = loadSource(geometryShaderPath, geometryOptional);

    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        return -1;
    }

    if (createProgram()) {
        return -1;
    }
    res = 0;
    return res;
}

GLint Shader::getUniformLoc(const std::string& name)
{
    if (uniforms.count(name)) {
        return uniforms[name];
    }

    GLint vi = glGetUniformLocation(id, name.c_str());
    uniforms[name] = vi;
    if (vi < 0) {
        std::cout << "SHADER::MISSING_UNIFORM: " << shaderName << " uniform=" << name << std::endl;
    }
    return vi;
}

int Shader::createProgram() {
    int success;
    char infoLog[512];

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
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED " << shaderName << " vert=" << vertexShaderPath << "\n" << infoLog << std::endl;
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
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED " << shaderName << " frag=" << fragmentShaderPath << "\n" << infoLog << std::endl;
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
            std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED " << shaderName << " frag=" << geometryShaderPath << "\n" << infoLog << std::endl;
        }
    }

    // link shaders
    id = glCreateProgram();
    glAttachShader(id, vertexShader);
    glAttachShader(id, fragmentShader);
    if (geometryShader != -1) {
        glAttachShader(id, geometryShader);
    }
    glLinkProgram(id);
    // check for linking errors
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED " << shaderName << "\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryShader != -1) {
        glDeleteShader(geometryShader);
    }

    // NOTE KI set UBOs only once for shader
    setUBO("Matrices", UBO_MATRICES);
    setUBO("Data", UBO_DATA);
    setUBO("Lights", UBO_LIGHTS);


    return 0;
}

void Shader::setFloat3(const std::string& name, float v1, float v2, float v3)
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform3f(vi, v1, v2, v3);
    }
}

void Shader::setVec3(const std::string& name, const glm::vec3& v)
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform3f(vi, v.x, v.y, v.z);
    }
}

void Shader::setVec4(const std::string& name, const glm::vec4& v)
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform4f(vi, v.x, v.y, v.z, v.w);
    }
}

void Shader::setFloat(const std::string& name, float value)
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform1f(vi, value);
    }
}

void Shader::setInt(const std::string& name, int value)
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform1i(vi, value);
    }
}

void Shader::setIntArray(const std::string& name, int count, const GLint* values)
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform1iv(vi, count, values);
    }
}

void Shader::setBool(const std::string& name, bool value)
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform1i(vi, (int)value);
    }
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat)
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniformMatrix4fv(vi, 1, GL_FALSE, glm::value_ptr(mat));
    }
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat)
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniformMatrix3fv(vi, 1, GL_FALSE, glm::value_ptr(mat));
    }
}

void Shader::setMat2(const std::string& name, const glm::mat2& mat)
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniformMatrix2fv(vi, 1, GL_FALSE, glm::value_ptr(mat));
    }
}

void Shader::setUBO(const std::string& name, unsigned int UBO)
{
    unsigned int blockIndex = glGetUniformBlockIndex(id, name.c_str());
    if (blockIndex == GL_INVALID_INDEX) {
        std::cout << "ERROR::SHADER::MISSING_UBO " << shaderName << " UBO=" << name << std::endl;
        return;
    } 
    glUniformBlockBinding(id, blockIndex, UBO);
}

/**
* Load shader file
*/
std::string Shader::loadSource(const std::string& path, bool optional) {
    std::string src;
    std::ifstream file;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(path);
        std::stringstream buf;
        buf << file.rdbuf();
        file.close();
        src = buf.str();
    } catch (std::ifstream::failure e) {
        if (!optional) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ " << shaderName << " path=" << path << std::endl;
        }
        else {
            std::cout << "INFO::SHADER::FILE_NOT_SUCCESFULLY_READ " << shaderName << " path=" << path << std::endl;
        }
    }
    std::cout << "\n== " << path << " ===\n" << src << "\n--------\n";

    return src;
}
